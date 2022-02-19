#include "metrics/IMetric.hpp"
#include "metrics/IMetricFamily.hpp"

IMetric::IMetric(IMetricFamily* family, LabelSet labelset) : _labelset(labelset), _parentFamily(family) {
}

std::string IMetric::getFullname() const {
	return _parentFamily->getFullname();
}

LabelSet IMetric::labelSet() const {
	return _labelset;
}