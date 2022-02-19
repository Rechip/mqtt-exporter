#include "metrics/Label.hpp"

std::string to_string(const LabelSet& labels) {
	if (labels.empty()) {
		return {};
	}

	std::string out;
	out += '{';
	for (const auto& [key, value] : labels) {
		out += key + "=\"" + value + "\",";
	}
	out.pop_back();
	out += '}';
	return out;
}