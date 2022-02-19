
#ifndef ME_APP_HPP_
#define ME_APP_HPP_

#include "Lockable.hpp"
#include "MosquittoClient.hpp"
#include "metrics/MetricFamily.hpp"
#include <CivetServer.h>
#include <shared_mutex>
#include <simple-yaml/simple_yaml.hpp>
#include <unordered_map>

struct LocalPrometheus : public simple_yaml::Simple {
	using Simple::Simple;

	std::string bind = bound("bind", "0.0.0.0:80,[::]:80");
};

using Metrics = std::map<std::string, std::shared_ptr<IMetricFamily>>;

class App {
public:
	using SharedMetrics = std::shared_ptr<Lockable<Metrics, std::shared_mutex>>;

	struct Configuration : simple_yaml::Simple {
		using Simple::Simple;

		MosquittoClient::Configuration            server  = bound("server");
		LocalPrometheus                           local   = bound("local");
		std::vector<IMetricFamily::Configuration> metrics = bound("metrics");
	};

	App(std::shared_ptr<rechip::logger::Logger> logger, Configuration config);
	App(const App&) = delete;
	App(App&&)      = delete;
	App& operator=(const App&) = delete;
	App& operator=(App&&) = delete;
	~App();

	void messageArrived(std::string topic, std::string payload, std::string omName) const;

protected:
	static std::unordered_map<std::string, std::string> parse(std::vector<Variable> variables, std::string topic, std::string payload);
	static LabelSet                                     replace(LabelSet labels, std::unordered_map<std::string, std::string> variables);

private:
	std::shared_ptr<rechip::logger::Logger>    _logger;
	Configuration                              _config;
	std::vector<std::shared_ptr<Subscription>> _subscriptions;
	std::unique_ptr<MosquittoClient>           _client;

	SharedMetrics                              _metrics;
	std::unique_ptr<CivetServer>               _server;
	std::vector<std::shared_ptr<CivetHandler>> _serverHandlers;
};

#endif
