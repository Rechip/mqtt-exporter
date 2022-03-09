#ifndef ME_METRICS_METRICFAMILY_HPP_
#define ME_METRICS_METRICFAMILY_HPP_
#pragma once

#include "IMetricFamily.hpp"
#include "Label.hpp"
#include "VectorIterator.hpp"
#include <map>
#include <mutex>
#include <ranges>
#include <type_traits>
#include <vector>

class IMetric;

template<typename MetricType>
requires std::is_base_of_v<IMetric, MetricType>
class MetricFamily : public IMetricFamily {
	struct MetricItem {
		std::shared_ptr<MetricType> metric;
		Clock::time_point           timestamp;
	};

public:
	MetricFamily(Configuration config) : IMetricFamily(std::move(config)) {
	}
	~MetricFamily() override = default;

	iterator begin() const override {
		std::scoped_lock lock{_mutex};
		auto             set = _metrics | std::views::values | std::views::transform([](const auto& item) { return item.metric; });
		return {{set.begin(), set.end()}, 0};
	}
	iterator end() const override {
		std::scoped_lock lock{_mutex};
		auto             set = _metrics | std::views::values | std::views::transform([](const auto& item) { return item.metric; });
		return {{set.begin(), set.end()}, set.size()};
	}

	void newSample(std::string_view payload, const LabelSet& labels) override {
		std::scoped_lock lock{_mutex};
		auto&            item = _metrics[labels];
		if (item.metric == nullptr) {
			item = MetricItem{.metric = std::make_shared<MetricType>(this, labels), .timestamp = {}};
		}
		item.timestamp = Clock::now();
		item.metric->newSample(payload);
	}

	OpenMetric::Type getStandardType() const override {
		return MetricType::standardType;
	}

	void routine() override {
		std::scoped_lock lock{_mutex};
		auto             now = Clock::now();
		for (auto it = _metrics.begin(); it != _metrics.end();) {
			if (now - it->second.timestamp > _config.maxAge) {
				it = _metrics.erase(it);
			} else {
				++it;
			}
		}
	}

protected:
	std::map<LabelSet, MetricItem> _metrics;
	mutable std::mutex             _mutex;
};

#endif
