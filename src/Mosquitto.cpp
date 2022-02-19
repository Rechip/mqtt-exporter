#include "Mosquitto.hpp"

#include <mosquitto.h>

Mosquitto::Mosquitto(const std::shared_ptr<rechip::Logger>& logger) {
	int major{};
	int minor{};
	int patch{};
	mosquitto_lib_version(&major, &minor, &patch);

	logger->debug() << "Mosquitto version: " << major << "." << minor << "." << patch;
	mosquitto_lib_init();
}

Mosquitto::~Mosquitto() {
	mosquitto_lib_cleanup();
}
