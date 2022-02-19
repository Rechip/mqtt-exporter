#include "metrics/Histogram.hpp"

#include <charconv>

Histogram::Histogram(IMetricFamily* family, LabelSet labelset)
    : IMetric(family, labelset), _count(std::make_shared<Counter::Point>(this)), _sum(std::make_shared<Gauge::Point>(this)) {
	for (double le : family->_config.openMetric.buckets) {
		_buckets.emplace_back(std::make_shared<Point>(this, le));
	}
	_buckets.emplace_back(std::make_shared<Point>(this, std::numeric_limits<double>::infinity()));

	std::sort(_buckets.begin(), _buckets.end(), [](const std::shared_ptr<Point>& a, const std::shared_ptr<Point>& b) { return a->_le > b->_le; });
}

void Histogram::newSample(std::string_view payload) {
	double sample{};
	std::from_chars(payload.begin(), payload.end(), sample);

	for (const auto& bucket : _buckets) {
		if (sample <= bucket->_le) {
			bucket->_value++;
		} else {
			break;
		}
	}
}

Histogram::iterator Histogram::begin() const {
	std::vector<std::shared_ptr<IPoint>> points{_buckets.begin(), _buckets.end()};
	points.emplace_back(_count);
	points.emplace_back(_sum);
	return {points, 0};
}

Histogram::iterator Histogram::end() const {
	std::vector<std::shared_ptr<IPoint>> points{_buckets.begin(), _buckets.end()};
	points.emplace_back(_count);
	points.emplace_back(_sum);
	return {points, points.size()};
}
