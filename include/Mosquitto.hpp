
#ifndef ME_MOSQUITTO_HPP_
#define ME_MOSQUITTO_HPP_
#pragma once

#include <rechip/logger.hpp>

class Mosquitto {
public:
	Mosquitto(const std::shared_ptr<rechip::Logger>& logger);
	Mosquitto(const Mosquitto&) = delete;
	Mosquitto(Mosquitto&&)      = delete;
	Mosquitto& operator=(const Mosquitto&) = delete;
	Mosquitto& operator=(Mosquitto&&) = delete;

	virtual ~Mosquitto();
};

#endif // ME_MOSQUITTO_CLIENT_HPP_
