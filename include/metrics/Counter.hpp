#ifndef ME_METRICS_COUNTER_HPP_
#define ME_METRICS_COUNTER_HPP_

#include "IMetric.hpp"
#include "IMetricFamily.hpp"

class Counter : public IMetric {
public:
	Counter(IMetricFamily* family, LabelSet labelset);
	virtual ~Counter() override = default;

	struct Point : IPoint {
		using IPoint::IPoint;
		virtual ~Point() = default;

		double value() const override {
			return _value;
		}

		std::string getFullname() const override {
			return _metric->getFullname() + "_total";
		}

		double _value{};
	};

	static constexpr OpenMetric::Type standardType = OpenMetric::Type::Counter;

	void     newSample(std::string_view payload) override;
	iterator begin() const override;
	iterator end() const override;

private:
	std::shared_ptr<Point> _point;
};

#endif
