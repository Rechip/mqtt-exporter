#include "metrics/MatchGauge.hpp"

#include <regex>
#include <string>

MatchGauge::MatchGauge(IMetricFamily* family, LabelSet labelset)
    : IMetric(family, labelset), _point(std::make_shared<Point>(this)), _matchRegex(std::regex(family->_config.openMetric.matchRegex)) {
}

void MatchGauge::newSample(std::string_view payload) {
	const std::ptrdiff_t match_count(std::distance(std::cregex_iterator(payload.begin(), payload.end(), _matchRegex), std::cregex_iterator()));

	_point->_value = match_count;
}

MatchGauge::iterator MatchGauge::begin() const {
	return {{_point}, 0};
}

MatchGauge::iterator MatchGauge::end() const {
	return {{_point}, 1};
}
