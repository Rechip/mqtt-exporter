#ifndef ME_METRICS_IMETRICFAMILY_HPP_
#define ME_METRICS_IMETRICFAMILY_HPP_
#pragma once

#include "IMetric.hpp"
#include "Label.hpp"
#include "Variable.hpp"
#include "VectorIterator.hpp"
#include <chrono>
#include <memory>
#include <simple-yaml/simple_yaml.hpp>
#include <string>
#include <string_view>
#include <vector>

struct OpenMetric : public simple_yaml::Simple {
	using Simple::Simple;

	enum class Type {
		//Standard types
		Counter,
		Gauge,
		Histogram,
		Summary,
		// Extension types
		MatchGauge
	};

	std::string        name   = bound("name");
	Type               type   = bound("type");
	std::string        help   = bound("help", "");
	std::string        unit   = bound("unit", "");
	std::vector<Label> labels = bound("labels");

	// Histogram specific
	std::vector<double> buckets = bound("buckets", std::vector<double>{});

	// Summary specific
	std::vector<double>  quantiles   = bound("quantiles", std::vector<double>{});
	std::chrono::seconds max_age     = bound("max_age", std::chrono::seconds{60});
	int                  age_buckets = bound("age_buckets", 5);

	// MatchGauge specific
	std::string matchRegex = bound("matchRegex", "");
};

class IMetricFamily {
public:
	struct Configuration : public simple_yaml::Simple {
		using Simple::Simple;

		std::string           topic      = bound("topic");
		OpenMetric            openMetric = bound("openMetric");
		std::vector<Variable> variables  = bound("variables");
	};

	using iterator = VectorIterator<std::shared_ptr<IMetric>>;

	IMetricFamily(Configuration config);
	virtual ~IMetricFamily() = default;

	static std::shared_ptr<IMetricFamily> create(Configuration config);

	std::string stringify() const;
	std::string getUnit() const;
	std::string getHelp() const;
	std::string getBaseName() const;
	std::string getTopic() const;
	std::string getFullname() const;

	std::vector<Variable> getVariables() const;

	LabelSet getConstantLabels() const;
	LabelSet getVariableLabels() const;

	virtual iterator         begin() const                                               = 0;
	virtual iterator         end() const                                                 = 0;
	virtual void             newSample(std::string_view payload, const LabelSet& labels) = 0;
	virtual OpenMetric::Type getStandardType() const                                     = 0;

	Configuration _config;
};

#endif
