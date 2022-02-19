#include "metrics/Gauge.hpp"

#include <charconv>
#include <ranges>

Gauge::Gauge(IMetricFamily* family, LabelSet labelset) : IMetric(family, labelset), _point(std::make_shared<Point>(this)) {
}

void Gauge::newSample(std::string_view payload) {
	std::from_chars(payload.begin(), payload.end(), _point->_value);
}

Gauge::iterator Gauge::begin() const {
	return {{_point}, 0};
}

Gauge::iterator Gauge::end() const {
	return {{_point}, 1};
}
