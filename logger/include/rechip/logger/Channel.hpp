#pragma once
#ifndef RECHIP_LOGGER_CHANNEL_HPP
#	define RECHIP_LOGGER_CHANNEL_HPP

#	include <source_location>
#	include <string>
#	include <unordered_map>
#	include "LogEntry.hpp"

namespace rechip::logger {

class Logger;

class Channel {
public:
	static constexpr const char CHANNEL[]     = "channel";
	static constexpr const char LEVEL[]       = "level";
	static constexpr const char LEVEL_TRACE[] = "trace";
	static constexpr const char LEVEL_DEBUG[] = "debug";
	static constexpr const char LEVEL_INFO[]  = "info";
	static constexpr const char LEVEL_WARN[]  = "warn";
	static constexpr const char LEVEL_ERROR[] = "error";
	static constexpr const char LEVEL_FATAL[] = "fatal";

	Channel(std::string channel, std::shared_ptr<Logger> logger = nullptr) : _logger(logger) {
		_fields[CHANNEL] = channel;
	}

	virtual std::shared_ptr<Logger> logger() {
		return _logger;
	}

	inline logger::LogEntry trace(std::source_location loc = std::source_location::current()) {
		auto fields   = _fields;
		fields[LEVEL] = LEVEL_TRACE;
		return LogEntry{logger(), fields, loc};
	}
	inline logger::LogEntry debug(std::source_location loc = std::source_location::current()) {
		auto fields   = _fields;
		fields[LEVEL] = LEVEL_DEBUG;
		return LogEntry{logger(), fields, loc};
	}
	inline logger::LogEntry info(std::source_location loc = std::source_location::current()) {
		auto fields   = _fields;
		fields[LEVEL] = LEVEL_INFO;
		return LogEntry{logger(), fields, loc};
	}
	inline logger::LogEntry warn(std::source_location loc = std::source_location::current()) {
		auto fields   = _fields;
		fields[LEVEL] = LEVEL_WARN;
		return LogEntry{logger(), fields, loc};
	}
	inline logger::LogEntry error(std::source_location loc = std::source_location::current()) {
		auto fields   = _fields;
		fields[LEVEL] = LEVEL_ERROR;
		return LogEntry{logger(), fields, loc};
	}
	inline logger::LogEntry fatal(std::source_location loc = std::source_location::current()) {
		auto fields   = _fields;
		fields[LEVEL] = LEVEL_FATAL;
		return LogEntry{logger(), fields, loc};
	}

	void setLoggerField(std::string key, std::string value) {
		_fields[key] = value;
	}

protected:
	std::shared_ptr<Logger>                      _logger;
	std::unordered_map<std::string, std::string> _fields;
};
} // namespace rechip::logger

#endif // !RECHIP_LOGGER_CHANNEL_HPP
