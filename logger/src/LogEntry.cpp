#include "rechip/logger/LogEntry.hpp"
#include "rechip/logger/Logger.hpp"

#include <chrono>
#include <iomanip>
#include <sstream>

using namespace rechip::logger;

LogEntry::LogEntry(
    std::shared_ptr<Logger>                             logger,
    const std::unordered_map<std::string, std::string>& fields,
    std::source_location                                loc,
    std::chrono::system_clock::time_point               timestamp)
    : _logger(logger), _timestamp(timestamp), _fields(fields), _loc(loc) {
}

LogEntry::~LogEntry() {
	_logger->log(*this);
}

bool LogEntry::hasField(const std::string& key) const {
	return _fields.contains(key);
}

std::string LogEntry::field(const std::string& key) const {
	if (key == "@timestamp") {
		std::stringstream ss;
		std::time_t       t  = std::chrono::system_clock::to_time_t(_timestamp);
		auto              lt = std::localtime(&t);

		ss << lt->tm_year + 1900 << "-";
		ss << std::setw(2) << std::setfill('0') << std::right << lt->tm_mon + 1 << "-";
		ss << std::setw(2) << std::setfill('0') << std::right << lt->tm_mday << " ";
		ss << std::setw(2) << std::setfill('0') << std::right << lt->tm_hour << ":";
		ss << std::setw(2) << std::setfill('0') << std::right << lt->tm_min << ":";
		ss << std::setw(2) << std::setfill('0') << std::right << lt->tm_sec << ".";
		ss << std::setw(6) << std::setfill('0') << std::right << duration_cast<std::chrono::microseconds>(_timestamp.time_since_epoch()).count() % 1'000'000;

		return ss.str();
	} else if (key == "@location") {
		std::stringstream ss;
		ss << _loc.file_name() << ":" << _loc.line();
		return ss.str();
	}
	return _fields.at(key);
}

void LogEntry::setField(const std::string& key, const std::string& value) {
	_fields[key] = value;
}
