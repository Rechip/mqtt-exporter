#pragma once
#ifndef RECHIP_LOGGER_FORMATTER_HPP_
#	define RECHIP_LOGGER_FORMATTER_HPP_

#	include <memory>
#	include <source_location>
#	include <unordered_map>
#	include <sstream>
#	include <chrono>

namespace rechip::logger {
class Logger;

class LogEntry : public std::stringstream {
public:
	LogEntry(
	    std::shared_ptr<Logger>                             logger,
	    const std::unordered_map<std::string, std::string>& fields,
	    std::source_location                                loc       = std::source_location::current(),
	    std::chrono::system_clock::time_point               timestamp = std::chrono::system_clock::now());
	LogEntry(const LogEntry&) = default;
	LogEntry(LogEntry&&)      = default;
	virtual ~LogEntry();

	bool        hasField(const std::string& name) const;
	std::string field(const std::string& name) const;
	void        setField(const std::string& name, const std::string& value);

private:
	std::unordered_map<std::string, std::string> _fields;
	std::source_location                         _loc;
	std::chrono::system_clock::time_point        _timestamp;
	std::shared_ptr<Logger>                      _logger;
};

} // namespace rechip::logger

#endif // RECHIP_LOGGER_FORMATTER_HPP_
