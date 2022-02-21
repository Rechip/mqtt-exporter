#include "metrics/IMetricFamily.hpp"

#include "metrics/Counter.hpp"
#include "metrics/Gauge.hpp"
#include "metrics/Histogram.hpp"
#include "metrics/IMetric.hpp"
#include "metrics/MatchGauge.hpp"
#include "metrics/MetricFamily.hpp"

#include <nlohmann/json.hpp>
#include <ranges>

IMetricFamily::IMetricFamily(Configuration config) : _config(std::move(config)) {
}

std::shared_ptr<IMetricFamily> IMetricFamily::create(IMetricFamily::Configuration config) {
	switch (config.openMetric.type) {
	case OpenMetric::Type::counter:
		return std::make_shared<MetricFamily<Counter>>(config);
	case OpenMetric::Type::gauge:
		return std::make_shared<MetricFamily<Gauge>>(config);
	case OpenMetric::Type::histogram:
		return std::make_shared<MetricFamily<Histogram>>(config);
	case OpenMetric::Type::MatchGauge:
		return std::make_shared<MetricFamily<MatchGauge>>(config);
	default:
		throw std::runtime_error("Unknown metric type");
	}
}

LabelSet IMetricFamily::getConstantLabels() const {
	auto result = _config.openMetric.labels | std::views::filter([](const Label& label) { return label.value.find('$') == std::string::npos; })
	              | std::views::transform([](const Label& label) { return std::make_pair(label.key, label.value); });
	return {result.begin(), result.end()};
}

LabelSet IMetricFamily::getVariableLabels() const {
	auto result = _config.openMetric.labels | std::views::filter([](const Label& label) { return label.value.find('$') != std::string::npos; })
	              | std::views::transform([](const Label& label) { return std::make_pair(label.key, label.value); });
	return {result.begin(), result.end()};
}

std::string IMetricFamily::stringify() const {
	std::string out;
	out += "# TYPE " + getFullname() + " " + std::string{magic_enum::enum_name(getStandardType())} + "\n";
	if (!getUnit().empty()) {
		out += "# UNIT " + getFullname() + " " + getUnit() + "\n";
	}
	out += "# HELP " + getFullname() + " " + getHelp() + "\n";

	for (auto metric : *this) {
		int i{0};
		for (auto point : *metric) {
			out += point->stringify();
			out += "\n";
			++i;
		}
		if (i > 1) {
			out += "\n";
		}
	}

	out += '\n';

	return out;
}

std::string IMetricFamily::getBaseName() const {
	return _config.openMetric.name;
}

std::string IMetricFamily::getTopic() const {
	return _config.topic;
}

std::vector<Variable> IMetricFamily::getVariables() const {
	return _config.variables;
}

std::string IMetricFamily::getHelp() const {
	return _config.openMetric.help;
}

std::string IMetricFamily::getUnit() const {
	return _config.openMetric.unit;
}

std::string IMetricFamily::getFullname() const {
	return getUnit().empty() ? getBaseName() : getBaseName() + "_" + getUnit();
}

std::string IMetricFamily::parsePayload(std::string payload) const {
	if (!_config.openMetric.payloadJsonPath.empty()) {
		nlohmann::json::json_pointer pointer{_config.openMetric.payloadJsonPath};
		nlohmann::json               json(nlohmann::json::parse(payload));
		payload = json.at(pointer).dump();
	}

	if (!_config.openMetric.payloadRegex.empty()) {
		std::regex  regex{_config.openMetric.payloadRegex};
		std::smatch match;
		if (std::regex_match(payload, match, regex)) {
			payload = match[1];
		}
	}

	return payload;
}
