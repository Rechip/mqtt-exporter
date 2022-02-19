#include "metrics/Counter.hpp"

Counter::Counter(IMetricFamily* family, LabelSet labelset) : IMetric(family, labelset), _point(std::make_shared<Point>(this)) {
}

void Counter::newSample([[maybe_unused]] std::string_view payload) {
	++(_point->_value);
}

Counter::iterator Counter::begin() const {
	return {{_point}, 0};
}

Counter::iterator Counter::end() const {
	return {{_point}, 1};
}
