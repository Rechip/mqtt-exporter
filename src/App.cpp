#include "App.hpp"

#include "Mosquitto.hpp"
#include "MosquittoClient.hpp"
#include <chrono>
#include <magic_enum.hpp>
#include <ranges>

using namespace std::chrono_literals;

class MetricsHandler : public CivetHandler {
public:
	MetricsHandler(App::SharedMetrics metrics) : _metrics(std::move(metrics)) {
		_output.reserve(4 * 1024 * 1024);
	}

	bool handleGet(CivetServer*, mg_connection* conn) override {
		mg_printf(
		    conn,
		    "HTTP/1.1 200 OK\r\n"
#ifdef _NDEBUG
		    "Content-Type: application/openmetrics-text; version=1.0.0; charset=utf-8\r\n"
#else
		    "Content-Type: text/plain; charset=utf-8\r\n"
#endif
		    "Connection: close\r\n\r\n");

		_output.clear();
		Metrics metrics;
		{
			std::shared_lock lock(*_metrics);
			metrics = *_metrics;
		}

		for (const auto& [name, metric] : metrics) {
			_output += metric->stringify();
			_output += "\n";
		}
		_output += "# EOF\n";

		mg_write(conn, _output.data(), _output.size());

		return true;
	}

private:
	App::SharedMetrics _metrics;
	std::string        _output;
};

App::App(std::shared_ptr<rechip::logger::Logger> logger, Configuration config)
    : _logger{logger}, _config{config}, _metrics{std::make_shared<Lockable<Metrics, std::shared_mutex>>()} {
	logger->info() << "Connect to " << config.server.address << " port " << config.server.port;
	logger->info() << "Local endpoint @ " << config.local.bind;
	logger->info() << "Metrics:";
	for (const auto& m : config.metrics) {
		logger->info() << "\ttopic: " << m.topic;
		logger->info() << "\ttype: " << magic_enum::enum_name(m.openMetric.type);
	}

	auto mosquitto = std::make_shared<Mosquitto>(logger);
	_client        = std::make_unique<MosquittoClient>(logger, mosquitto, config.server);

	auto connTicket = _client->connect();
	if (!connTicket->wait_for(5s)) {
		throw RuntimeErrorException("Could not connect to the server in 5s");
	}

	for (const auto& m : config.metrics) {
		auto subscription = _client->subscribe(m.topic);
		subscription->onArrived.connect([this, omName = m.openMetric.name](const Message& m) { messageArrived(m.topic, m.payload, omName); });
		_subscriptions.emplace_back(subscription);

		auto metric = IMetricFamily::create(m);
		if (metric != nullptr) {
			std::unique_lock lock{*_metrics};
			_metrics->emplace(m.topic, metric);
		}
	}

	_server = std::make_unique<CivetServer>(
	    std::vector<std::string>{"listening_ports", config.local.bind, "additional_header", "X-Author:Rechip", "num_threads", "2"});
	auto handler = std::make_shared<MetricsHandler>(_metrics);
	_server->addHandler("/metrics", handler.get());
	_serverHandlers.emplace_back(handler);

	_routineThread = std::jthread([this](std::stop_token token) {
		while (!token.stop_requested()) {
			{
				std::scoped_lock lock{*_metrics};
				for (auto& [name, metric] : *_metrics) {
					metric->routine();
				}
			}

			std::this_thread::sleep_for(1s);
		}
	});
}

App::~App() {
	_routineThread.request_stop();
	if (_routineThread.joinable()) {
		_routineThread.join();
	}

	auto ticket = _client->disconnect();
	_server->close();
	ticket->wait_for(1s);
	_client.reset();
}

void App::messageArrived(std::string topic, std::string payload, std::string omName) const {
	std::vector<std::shared_ptr<IMetricFamily>> metrics;
	{
		std::shared_lock lock(*_metrics);
		/*auto             matchingMetrics = *_metrics | std::views::filter([&topic, this](const std::pair<std::string, std::shared_ptr<IMetricFamily>>& mp) {
            return _client->matchTopic(mp.first, topic);
        }) | std::views::values;
		metrics                          = std::vector<std::shared_ptr<IMetricFamily>>{matchingMetrics.begin(), matchingMetrics.end()};*/
		for (const auto& [name, metric] : *_metrics) {
			if (_client->matchTopic(name, topic) && metric->getBaseName() == omName) {
				metrics.emplace_back(metric);
			}
		}
	}

	for (const auto& metric : metrics) {
		LabelSet variableLabels(metric->getVariableLabels());

		auto parsed    = parse(metric->getVariables(), topic, payload);
		variableLabels = replace(variableLabels, parsed);
		// TODO: parse payload by regex or JSON path
		auto parsedPayload = metric->parsePayload(payload);
		metric->newSample(parsedPayload, variableLabels);
	}
}

std::unordered_map<std::string, std::string> App::parse(std::vector<Variable> variables, std::string topic, std::string payload) {
	std::unordered_map<std::string, std::string> result;

	for (const auto& v : variables) {
		std::string source;

		switch (v.target) {
		case Variable::Target::topic:
			source = topic;
			break;
		case Variable::Target::payload:
			source = payload;
			break;
		}

		// TODO: allow also JSON Path
		std::regex  regex{v.regex};
		std::smatch match;

		result[v.name] = std::regex_search(source, match, regex) ? match[1].str() : "";
	}

	return result;
}

std::string replaceAll(std::string str, const std::string& from, const std::string& to) {
	size_t start_pos = 0;
	while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
		str.replace(start_pos, from.length(), to);
		start_pos += to.length(); // Handles case where 'to' is a substring of 'from'
	}
	return str;
}

LabelSet App::replace(LabelSet labels, std::unordered_map<std::string, std::string> variables) {
	for (const auto& [name, value] : variables) {
		std::string fullname = "${" + name + "}";

		for (auto& [k, v] : labels) {
			v = replaceAll(v, fullname, value);
		}
	}
	return labels;
}
