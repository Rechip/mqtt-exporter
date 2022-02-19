#ifndef ME_METRICS_METRICFAMILY_HPP_
#define ME_METRICS_METRICFAMILY_HPP_
#pragma once

#include "IMetricFamily.hpp"
#include "Label.hpp"
#include "VectorIterator.hpp"
#include <map>
#include <ranges>
#include <type_traits>
#include <vector>

class IMetric;

template<typename MetricType>
requires std::is_base_of_v<IMetric, MetricType>
class MetricFamily : public IMetricFamily {
public:
	MetricFamily(Configuration config) : IMetricFamily(std::move(config)) {
	}
	~MetricFamily() override = default;

	iterator begin() const override {
		auto set = _metrics | std::views::values;
		return {{set.begin(), set.end()}, 0};
	}
	iterator end() const override {
		auto set = _metrics | std::views::values;
		return {{set.begin(), set.end()}, set.size()};
	}

	void newSample(std::string_view payload, const LabelSet& labels) override {
		std::shared_ptr<MetricType> metric = _metrics[labels];
		if (metric == nullptr) {
			metric           = std::make_shared<MetricType>(this, labels);
			_metrics[labels] = metric;
		}
		metric->newSample(payload);
	}

	OpenMetric::Type getStandardType() const override {
		return MetricType::standardType;
	}

protected:
	std::map<LabelSet, std::shared_ptr<MetricType>> _metrics;
};

#endif
