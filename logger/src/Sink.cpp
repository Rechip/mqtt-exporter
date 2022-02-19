#include "rechip/logger/Sink.hpp"
#include "rechip/logger/LogEntry.hpp"
#include "rechip/logger/Logger.hpp"

#include <sstream>

constexpr auto bgBlack     = "\033[40m";
constexpr auto bgRed       = "\033[41m";
constexpr auto bgGreen     = "\033[42m";
constexpr auto bgOrange    = "\033[43m";
constexpr auto bgBlue      = "\033[44m";
constexpr auto bgMagenta   = "\033[45m";
constexpr auto bgCyan      = "\033[46m";
constexpr auto bgLightGray = "\033[47m";
constexpr auto bgDefault   = "\033[49m";

constexpr auto bgYellow      = "\033[103m";
constexpr auto bgDarkGray    = "\033[100m";
constexpr auto bgLightRed    = "\033[101m";
constexpr auto bgLightGreen  = "\033[102m";
constexpr auto bgLightBlue   = "\033[104m";
constexpr auto bgLightPurple = "\033[105m";
constexpr auto bgTeal        = "\033[106m";
constexpr auto bgWhite       = "\033[107m";

constexpr auto fgBlack     = "\033[30m";
constexpr auto fgRed       = "\033[31m";
constexpr auto fgGreen     = "\033[32m";
constexpr auto fgOrange    = "\033[33m";
constexpr auto fgBlue      = "\033[34m";
constexpr auto fgMagenta   = "\033[35m";
constexpr auto fgCyan      = "\033[36m";
constexpr auto fgLightGray = "\033[37m";
constexpr auto fgDefault   = "\033[39m";

using namespace rechip::logger;

void Sink::write(const LogEntry& entry) {
	write(_formatter(entry));
}

std::string Sink::defaultFormatter(const LogEntry& entry) {
	std::stringstream ss;

	ss << "[" << entry.field("@timestamp") << "] ";

	if (entry.hasField(Channel::CHANNEL)) {
		ss << "[" << entry.field(Channel::CHANNEL) << "] ";
	}

	if (entry.hasField(Channel::LEVEL)) {
		auto level = entry.field(Channel::LEVEL);
		if (level == Channel::LEVEL_TRACE) {
			ss << bgDarkGray << fgBlack << " trace " << fgDefault << bgDefault;
			ss << " " << fgLightGray << entry.str() << fgDefault;
			ss << " @ " << entry.field("@location") << std::endl;
		} else if (level == Channel::LEVEL_DEBUG) {
			ss << bgWhite << fgBlack << " debug " << fgDefault << bgDefault;
			ss << " " << fgLightGray << entry.str() << fgDefault << std::endl;
		} else if (level == Channel::LEVEL_INFO) {
			ss << bgLightGreen << fgBlack << " info " << fgDefault << bgDefault;
			ss << " " << fgGreen << entry.str() << fgDefault << std::endl;
		} else if (level == Channel::LEVEL_WARN) {
			ss << bgYellow << fgBlack << " warning " << fgDefault << bgDefault;
			ss << " " << fgOrange << entry.str() << fgDefault << std::endl;
		} else if (level == Channel::LEVEL_ERROR) {
			ss << bgRed << fgBlack << " error " << fgDefault << bgDefault;
			ss << " " << fgRed << entry.str() << fgDefault;
			ss << " @ " << entry.field("@location") << std::endl;
		} else if (level == Channel::LEVEL_FATAL) {
			ss << bgRed << fgBlack << " fatal " << fgDefault << bgDefault;
			ss << " " << bgRed << fgBlack << entry.str() << fgDefault << bgDefault;
			ss << " @ " << entry.field("@location") << std::endl;
		}
	}

	return ss.str();
}
