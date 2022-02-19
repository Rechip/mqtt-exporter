#pragma once
#ifndef RECHIP_LOGGER_SINK_HPP_
#	define RECHIP_LOGGER_SINK_HPP_

#	include <functional>
#	include <memory>
#	include <iostream>
#	include <mutex>
#	include <string>

namespace rechip::logger {

class LogEntry;

class Sink {
public:
	virtual void write(std::string data) = 0;
	void         write(const LogEntry& entry);

	virtual ~Sink() = default;

	using Formatter = std::function<std::string(const LogEntry&)>;

	void setFormatter(Formatter formatter) {
		_formatter = formatter;
	}

	static std::string defaultFormatter(const LogEntry&);

private:
	Formatter _formatter{defaultFormatter};
};

class ClogSink : public Sink {
public:
	static std::shared_ptr<rechip::logger::Sink> pointer() {
		static auto instance = std::make_shared<ClogSink>();
		return instance;
	}

	void write(std::string data) override {
		std::scoped_lock lock{_mutex};
		std::clog << data;
		std::clog.flush();
	}

private:
	std::mutex _mutex;
};

class CoutSink : public Sink {
public:
	static std::shared_ptr<rechip::logger::Sink> pointer() {
		static auto instance = std::make_shared<CoutSink>();
		return instance;
	}

	void write(std::string data) override {
		std::scoped_lock lock{_mutex};
		std::cout << data;
		std::cout.flush();
	}

private:
	std::mutex _mutex;
};

class CerrSink : public Sink {
public:
	static std::shared_ptr<rechip::logger::Sink> pointer() {
		static auto instance = std::make_shared<CerrSink>();
		return instance;
	}

	void write(std::string data) override {
		std::scoped_lock lock{_mutex};
		std::cerr << data;
		std::cerr.flush();
	}

private:
	std::mutex _mutex;
};

} // namespace rechip::logger

#endif // RECHIP_LOGGER_SINK_HPP_
