#pragma once
#ifndef RECHIP_LOGGER_LOGGER_HPP_
#	define RECHIP_LOGGER_LOGGER_HPP_

#	include <memory>
#	include <sstream>
#	include <source_location>
#	include <iostream>
#	include <mutex>
#	include <map>
#	include <cassert>

#	include "LogEntry.hpp"
#	include "Sink.hpp"
#	include "Channel.hpp"

namespace rechip::logger {

class Logger : public std::enable_shared_from_this<Logger>, public Channel {
public:
	using SinkName = std::string;
	using Router   = std::function<SinkName(const LogEntry&)>;

	Logger();

	std::shared_ptr<Logger> logger() override;

	std::shared_ptr<logger::Sink> setSink(const std::string& name, std::shared_ptr<logger::Sink> sink);
	std::shared_ptr<logger::Sink> sink(const std::string& name) const;
	void                          addRouter(Router router, int order = 0);
	void                          clearRouters();
	SinkName                      route(const LogEntry& entry) const;
	void                          log(const LogEntry& entry);
	Channel                       channel(const std::string& name);

private:
	std::multimap<int, Router>                                     _routers;
	std::unordered_map<std::string, std::shared_ptr<logger::Sink>> _sinks;
};

} // namespace rechip::logger

#endif // RECHIP_LOGGER_LOGGER_HPP_
