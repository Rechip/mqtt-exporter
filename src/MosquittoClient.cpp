#include "MosquittoClient.hpp"

#include "MosquittoException.hpp"
#include <chrono>
#include <iostream>
#include <mosquitto.h>
#include <ranges>

using namespace std::chrono_literals;

MosquittoClient::MosquittoClient(const std::shared_ptr<rechip::logger::Logger>& logger, std::shared_ptr<Mosquitto> mosquitto, Configuration configuration)
    : Channel{logger->channel("MosquittoClient")}, _mosquitto{std::move(mosquitto)}, _config{std::move(configuration)}, _disconnectTicket{
                                                                                                                            std::make_shared<Ticket>()} {
	setLoggerField("id", _config.clientId);

	trace() << "mosquitto_new";
	_instance = mosquitto_new(_config.clientId.empty() ? nullptr : _config.clientId.c_str(), true, this);
	if (_instance == nullptr) {
		throw MosquittoException("Can not create a mosquitto instance");
	}
	if (auto ret = mosquitto_threaded_set(_instance, true); ret != MOSQ_ERR_SUCCESS) {
		throw MosquittoException("Can not set thread option");
	}

	mosquitto_int_option(_instance, MOSQ_OPT_PROTOCOL_VERSION, MQTT_PROTOCOL_V311);
	if (_config.tls) {
		mosquitto_int_option(_instance, MOSQ_OPT_TLS_USE_OS_CERTS, 1); // This is important to verify the cert properly
	}

	mosquitto_connect_callback_set(_instance, onConnected);
	mosquitto_publish_callback_set(_instance, onPublished);
	mosquitto_disconnect_callback_set(_instance, onDisconnected);
	mosquitto_message_callback_set(_instance, onMessage);
	mosquitto_log_callback_set(_instance, onLog);

	_worker = std::jthread([this](std::stop_token token) {
		while (!token.stop_requested()) {
			mosquitto_loop(_instance, 100, 1);
			std::this_thread::sleep_for(10ms);
		}
	});
}

MosquittoClient::~MosquittoClient() {
	trace() << "~MosquittoClient";
	if (_disconnectTicket && !_disconnectTicket->wait_for(std::chrono::milliseconds(0))) {
		auto ticket = disconnect();
		ticket->wait_for(3s);
	}

	_worker.request_stop();
	if (_worker.joinable()) {
		_worker.join();
	}

	trace() << "mosquitto_destroy";
	mosquitto_destroy(_instance);
}

std::shared_ptr<Ticket> MosquittoClient::connect() {
	_connectTicket = std::make_shared<Ticket>();

	debug() << "Connecting to " << _config.address << ":" << _config.port;

	if (!_config.username.empty()) {
		debug() << "\tcredentials" << _config.username << ":" << _config.password;
		auto retVal = mosquitto_username_pw_set(_instance, _config.username.c_str(), _config.password.c_str());
		if (retVal != MOSQ_ERR_SUCCESS) {
			throw ClientError("Can not set a username + password");
		}
	}

	auto retVal = mosquitto_connect_async(_instance, _config.address.c_str(), _config.port, _config.keepAlive);
	if (retVal != MOSQ_ERR_SUCCESS) {
		trace() << "Can not connect";
		throw ClientError(
		    "Can not connect to " + _config.address + ":" + std::to_string(_config.port) + " " + _config.username + ":" + _config.password
		    + " errcode:" + mosquitto_strerror(retVal));
	}

	return _connectTicket;
}

std::shared_ptr<Ticket> MosquittoClient::disconnect() {
	_disconnectTicket = std::make_shared<Ticket>();
	debug() << "Disconnecting";
	auto retVal = mosquitto_disconnect(_instance);
	if (retVal != MOSQ_ERR_SUCCESS) {
		trace() << "Can not disconnect? " << mosquitto_strerror(retVal) << " " << retVal;
		throw ClientError(std::string{"Can not disconnect, errcode:"} + mosquitto_strerror(retVal));
	}
	return _disconnectTicket;
}

std::shared_ptr<Ticket> MosquittoClient::publish(const std::string& topic, const std::string& data) {
	int mid{};
	debug() << "Publishing " << data << " into " << topic;
	auto retval = mosquitto_publish(_instance, &mid, topic.c_str(), static_cast<int>(data.length()), static_cast<const void*>(data.c_str()), 1, true);
	if (retval != MOSQ_ERR_SUCCESS) {
		trace() << "Error publishing";
		throw ClientError("Can not publish message to " + topic);
	}

	std::scoped_lock lock{_messageTicketsMutex};

	if (_messageTickets.contains(mid)) {
		trace() << "Error publishing, duplicate";
		throw ClientError("Generated a duplicate messageId that is currently on-the-fly");
	}

	auto ticket          = std::make_shared<Ticket>();
	_messageTickets[mid] = ticket;
	return ticket;
}

std::shared_ptr<Subscription> MosquittoClient::subscribe(const std::string& topic) {
	auto liveSubscriptions = _subscriptions | std::views::transform([](const std::weak_ptr<Subscription>& wptr) { return wptr.lock(); })
	                         | std::views::filter([](const std::shared_ptr<Subscription>& sptr) { return sptr != nullptr; });
	auto alreadySubscribed = std::ranges::find_if(liveSubscriptions, [&topic](const std::shared_ptr<Subscription>& sptr) { return sptr->topic == topic; });
	if (alreadySubscribed != std::ranges::end(liveSubscriptions)) {
		return *alreadySubscribed;
	}

	auto s   = std::make_shared<Subscription>(this);
	s->topic = topic;
	int mid{};
	mosquitto_subscribe(_instance, &mid, topic.c_str(), 0);
	//TODO: ticket and such

	_subscriptions.push_back(s);

	return s;
}

void MosquittoClient::onConnected(mosquitto*, void* data, int) {
	auto* This = static_cast<MosquittoClient*>(data);
	This->debug() << "connected";

	This->_connectTicket->trigger();
}

void MosquittoClient::onDisconnected(mosquitto*, void* data, int reason) {
	auto* This = static_cast<MosquittoClient*>(data);
	This->debug() << "disconnected: " << reason;

	This->_disconnectTicket->trigger();
}

void MosquittoClient::onPublished(mosquitto*, void* data, int mid) {
	auto* This = static_cast<MosquittoClient*>(data);
	This->trace() << "published " << mid;

	std::scoped_lock lock{This->_messageTicketsMutex};
	auto&            tickets = This->_messageTickets;

	if (!tickets.contains(mid)) {
		return;
	}

	auto ticket = tickets.at(mid).lock();
	tickets.erase(mid);
	if (ticket == nullptr) {
		return;
	}

	ticket->trigger();
}

void MosquittoClient::onMessage(mosquitto*, void* data, const mosquitto_message* message) {
	auto*   This = static_cast<MosquittoClient*>(data);
	Message m{message};

	auto subs = This->_subscriptions | std::views::transform([](const std::weak_ptr<Subscription>& wptr) { return wptr.lock(); })
	            | std::views::filter([](const std::shared_ptr<Subscription>& sptr) { return sptr != nullptr; });
	for (auto s : subs) {
		if (matchTopic(s->topic, m.topic)) {
			s->onArrived(m);
		}
	}
}

void MosquittoClient::onLog(mosquitto*, void* data, int level, const char* msg) {
	auto* This = static_cast<MosquittoClient*>(data);
	This->trace() << "Log(" << level << "): " << msg;
}

bool MosquittoClient::matchTopic(const std::string& subscription, const std::string& topic) {
	bool ret{false};
	auto res = mosquitto_topic_matches_sub(subscription.c_str(), topic.c_str(), &ret);
	switch (res) {
	case MOSQ_ERR_INVAL:
		throw MosquittoException("Invalid input to matchTopic");
	case MOSQ_ERR_NOMEM:
		throw MosquittoException("Not enough memory for matchTopic");
	case MOSQ_ERR_SUCCESS:
		break;
	default:
		throw MosquittoException("Unknown exception");
	}

	return ret;
}
