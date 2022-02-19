#include "rechip/logger/Logger.hpp"

using namespace rechip::logger;

Logger::Logger() : Channel{"!General"} {
	setSink("@cout", logger::CoutSink::pointer());
	setSink("@clog", logger::ClogSink::pointer());
	setSink("@cerr", logger::CerrSink::pointer());

	// default rule to log all messages without a rule
	addRouter([](const LogEntry& entry) { return "@clog"; }, 100);

	// if a record has a "level" and the level is a error type
	addRouter(
	    [](const LogEntry& entry) {
		    if (entry.hasField(Channel::LEVEL)) {
			    if (auto level = entry.field(Channel::LEVEL); level == Channel::LEVEL_FATAL || level == Channel::LEVEL_ERROR) {
				    return "@cerr";
			    }
		    }
		    return "";
	    },
	    10);
}

std::shared_ptr<Logger> Logger::logger() {
	return shared_from_this();
}

std::shared_ptr<Sink> Logger::setSink(const std::string& name, std::shared_ptr<Sink> sink) {
	_sinks[name] = sink;
	return sink;
}

std::shared_ptr<Sink> Logger::sink(const std::string& name) const {
	return _sinks.at(name);
}

void Logger::addRouter(Router router, int order) {
	_routers.emplace(order, router);
}

void Logger::clearRouters() {
	_routers.clear();
}

Logger::SinkName Logger::route(const LogEntry& entry) const {
	for (const auto& r : _routers) {
		if (auto name = r.second(entry); !name.empty()) {
			return name;
		}
	}
	return "";
}

void Logger::log(const LogEntry& entry) {
	if (auto name = route(entry); !name.empty()) {
		if (_sinks.contains(name)) {
			auto sink = _sinks.at(name);
			sink->write(entry);
		} else {
			throw std::runtime_error("No sink found for name: " + name);
		}
	} else {
		throw std::runtime_error("Sending it to the empty sink");
	}
}

Channel Logger::channel(const std::string& name) {
	return Channel{name, shared_from_this()};
}
