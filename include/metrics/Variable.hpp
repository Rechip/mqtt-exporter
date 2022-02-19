#ifndef ME_METRICS_VARIABLE_HPP_
#define ME_METRICS_VARIABLE_HPP_
#pragma once

#include <simple-yaml/simple_yaml.hpp>
#include <string>

struct Variable : public simple_yaml::Simple {
	using Simple::Simple;

	enum class Target { topic, payload };

	std::string name   = bound("name");
	Target      target = bound("target");
	std::string regex  = bound("regex");
};

#endif
