
#include <atomic>
#include <csignal>
#include <cstdlib>
#include <iostream>
#include <rechip/logger.hpp>
#include <simple-yaml/simple_yaml.hpp>

#include "App.hpp"
#include "Exception.hpp"

std::atomic_bool shouldTerminate{false};

void sigHandle(int sig) {
	std::clog << "Received " << sig << ", terminating..." << std::endl;
	shouldTerminate = true;
}

int main() {
	std::ios_base::sync_with_stdio(false);
	signal(SIGTERM, sigHandle); // on Docker, SIGTERM is sent when the container is stopped
	signal(SIGINT, sigHandle);

	auto logger = std::make_shared<rechip::Logger>();

	logger->info() << "Welcome to the finest mqtt-exporter!";

	try {
		App::Configuration config{simple_yaml::fromFile("mqtt-exporter.yml")};

		App app(logger, config);

#ifdef _NDEBUG
		logger->info() << "Press ENTER to end the program";
		std::cin.get();
#else
		logger->info() << "Press CTRL+C to end the program";
		while (!shouldTerminate) {
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
		}
#endif
		logger->info() << "Terminating...";

	} catch (const YAML::Exception& e) {
		logger->error() << "Unable to open and parse a configuration file";
		logger->debug() << e.what();
		return EXIT_FAILURE;
	} catch (const simple_yaml::RuntimeError& e) {
		logger->error() << "Bad file structure";
		logger->debug() << e;
		return EXIT_FAILURE;
	} catch (const Exception& e) {
		logger->error() << "Exception thrown";
		logger->debug() << e;
	}
}
