#pragma once
#ifndef ME_MOSQUITTO_CLIENT_HPP_
#	define ME_MOSQUITTO_CLIENT_HPP_

#	include "Exception.hpp"
#	include "Signal.hpp"
#	include <chrono>
#	include <future>
#	include <memory>
#	include <rechip/logger/Channel.hpp>
#	include <rechip/logger/Logger.hpp>
#	include <simple-yaml/simple_yaml.hpp>
#	include <thread>
#	include <unordered_map>
#	include <mosquitto.h>

class Mosquitto;
struct mosquitto;

using ClientError = BaseException<std::runtime_error>;

struct Ticket {
	Ticket() : _waitingPoint{_promise.get_future()} {
	}

	template<typename Rep, typename Period>
	bool wait_for(std::chrono::duration<Rep, Period> duration) {
		return _waitingPoint.wait_for(duration) != std::future_status::timeout;
	}

	void trigger() {
		if (_waitingPoint.wait_for(std::chrono::milliseconds(0)) != std::future_status::ready) {
			_promise.set_value();
		}
	}

private:
	std::promise<void> _promise;
	std::future<void>  _waitingPoint;
};

struct Message {
	int         mid{0};
	std::string topic;
	std::string payload;
	int         qos{0};
	bool        retain{false};

	Message(const mosquitto_message* m)
	    : mid{m->mid}, topic{m->topic}, payload{static_cast<char*>(m->payload), static_cast<size_t>(m->payloadlen)}, qos{m->qos}, retain{m->retain} {
	}
};

struct Subscription;

class MosquittoClient : public rechip::logger::Channel {
public:
	struct Configuration : simple_yaml::Simple {
		using Simple::Simple;

		uint16_t    port      = bound("port");
		std::string address   = bound("address");
		std::string clientId  = bound("clientId", "");
		std::string username  = bound("username", "");
		std::string password  = bound("password", "");
		bool        tls       = bound("tls", false);
		int         keepAlive = bound("keepAlive", 60);
	};

	MosquittoClient(const std::shared_ptr<rechip::logger::Logger>& logger, std::shared_ptr<Mosquitto> mosquitto, Configuration configuration);
	virtual ~MosquittoClient();

	[[nodiscard]] std::shared_ptr<Ticket> connect();
	[[nodiscard]] std::shared_ptr<Ticket> disconnect();
	std::shared_ptr<Ticket>               publish(const std::string& topic, const std::string& data);
	std::shared_ptr<Subscription>         subscribe(const std::string& topic);

	static bool matchTopic(const std::string& subscription, const std::string& topic);

protected:
	static void onConnected(mosquitto*, void*, int);
	static void onDisconnected(mosquitto*, void*, int);
	static void onPublished(mosquitto*, void*, int);
	static void onMessage(mosquitto*, void*, const mosquitto_message*);
	static void onLog(mosquitto*, void*, int, const char*);

private:
	std::shared_ptr<Mosquitto> _mosquitto;
	Configuration              _config;
	mosquitto*                 _instance{};
	std::jthread               _worker;

	std::mutex                                     _messageTicketsMutex;
	std::unordered_map<int, std::weak_ptr<Ticket>> _messageTickets;
	std::vector<std::weak_ptr<Subscription>>       _subscriptions;

	std::shared_ptr<Ticket> _connectTicket;
	std::shared_ptr<Ticket> _disconnectTicket;
};

struct Subscription {
	std::string            topic;
	Zeeno::Signal<Message> onArrived;

	Subscription([[maybe_unused]] MosquittoClient* client) {
	}

private:
};

#endif // ME_MOSQUITTO_CLIENT_HPP_