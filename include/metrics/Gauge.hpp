#ifndef ME_METRICS_GAUGE_HPP_
#define ME_METRICS_GAUGE_HPP_

#include "IMetric.hpp"
#include "IMetricFamily.hpp"

class Gauge : public IMetric {
public:
	Gauge(IMetricFamily* family, LabelSet labelset);
	~Gauge() override = default;

	struct Point : public IPoint {
		using IPoint::IPoint;
		virtual ~Point() = default;

		double value() const override {
			return _value;
		}

		double _value;
	};

	static constexpr OpenMetric::Type standardType = OpenMetric::Type::Gauge;

	void     newSample(std::string_view payload) override;
	iterator begin() const override;
	iterator end() const override;

private:
	std::shared_ptr<Point> _point;
};

#endif
