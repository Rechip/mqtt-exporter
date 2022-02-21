#ifndef ME_METRICS_BINARY_GAUGE_HPP_
#define ME_METRICS_BINARY_GAUGE_HPP_

#include "MetricFamily.hpp"
#include <regex>

class MatchGauge : public IMetric {
public:
	MatchGauge(IMetricFamily* family, LabelSet labelset);
	~MatchGauge() override = default;

	struct Point : public IPoint {
		using IPoint::IPoint;
		virtual ~Point() = default;

		double value() const override {
			return _value;
		}

		double _value;
	};

	static constexpr OpenMetric::Type standardType = OpenMetric::Type::gauge;

	void     newSample(std::string_view payload) override;
	iterator begin() const override;
	iterator end() const override;

private:
	std::shared_ptr<Point> _point;
	std::regex             _matchRegex;
};

#endif
