#ifndef ME_METRICS_LABEL_HPP_
#define ME_METRICS_LABEL_HPP_
#pragma once

#include <map>
#include <simple-yaml/simple_yaml.hpp>
#include <string>

//TODO: why is it here?
struct Label : public simple_yaml::Simple {
	using Simple::Simple;

	using key_t   = std::string;
	using value_t = std::string;

	key_t   key   = bound("key");
	value_t value = bound("value");
};

using LabelSet = std::map<Label::key_t, Label::value_t>;
std::string to_string(const LabelSet& labels);

#endif
