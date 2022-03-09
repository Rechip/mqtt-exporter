#ifndef ME_METRICS_IMETRIC_HPP_
#define ME_METRICS_IMETRIC_HPP_
#pragma once

#include "Label.hpp"
#include "VectorIterator.hpp"
#include <chrono>

class IMetricFamily;

class IMetric {
public:
	class IPoint {
	public:
		using Clock = std::chrono::system_clock;

		IPoint(IMetric* metric) : _metric(metric) {
		}
		virtual ~IPoint() = default;

		virtual double value() const = 0;

		virtual LabelSet labelSet() const {
			return _metric->labelSet();
		}
		virtual std::string getFullname() const {
			return _metric->getFullname();
		}

		std::string stringify() const {
			std::stringstream ss;
			ss << getFullname() << to_string(labelSet()) << " " << value();
			return ss.str();
		}

	protected:
		IMetric* _metric{};
	};

	IMetric(IMetricFamily* family, LabelSet labelset);
	virtual ~IMetric() = default;

	using iterator = VectorIterator<std::shared_ptr<IPoint>>;
	using Clock    = IPoint::Clock;

	virtual iterator begin() const                       = 0;
	virtual iterator end() const                         = 0;
	virtual void     newSample(std::string_view payload) = 0;

	virtual std::string getFullname() const;
	virtual LabelSet    labelSet() const;

protected:
	LabelSet       _labelset;
	IMetricFamily* _parentFamily{};
};

template<typename MetricPointType>
requires std::is_base_of_v<IMetric::IPoint, MetricPointType>
class Metric {
public:
	virtual ~Metric() = default;
};

#endif
