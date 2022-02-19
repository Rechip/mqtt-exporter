#ifndef ME_METRICS_HISTOGRAM_HPP_
#define ME_METRICS_HISTOGRAM_HPP_

#include "Counter.hpp"
#include "Gauge.hpp"
#include "IMetric.hpp"
#include "IMetricFamily.hpp"

class Histogram : public IMetric {
public:
	Histogram(IMetricFamily* family, LabelSet labelset);
	virtual ~Histogram() override = default;

	struct Point : public IPoint {
		Point(Histogram* histogram, double le) : IPoint(histogram), _le(le) {
			_labelSet = _metric->labelSet();
			_labelSet.insert({"le", _le == std::numeric_limits<double>::infinity() ? "+Inf" : std::to_string(_le)});
		}

		virtual ~Point() = default;

		double value() const override {
			return _value;
		}

		std::string getFullname() const override {
			return _metric->getFullname() + "_bucket";
		}

		LabelSet labelSet() const override {
			return _labelSet;
		}

		double   _value{};
		double   _le;
		LabelSet _labelSet;
	};

	static constexpr OpenMetric::Type standardType = OpenMetric::Type::Histogram;

	void                newSample(std::string_view payload) override;
	Histogram::iterator begin() const override;
	Histogram::iterator end() const override;

private:
	std::shared_ptr<Counter::Point>     _count;
	std::shared_ptr<Gauge::Point>       _sum;
	std::vector<std::shared_ptr<Point>> _buckets;
};

#endif
