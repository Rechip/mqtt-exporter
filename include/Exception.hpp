#pragma once
#ifndef ME_EXCEPTION_HPP_
#	define ME_EXCEPTION_HPP_

#	include <source_location>
#	include <stdexcept>
#	include <ostream>

class Exception : public std::exception {
public:
	virtual std::string location() const = 0;
};

template<typename Parent>
class BaseException : public Exception, public Parent {
public:
	BaseException(const std::string& what, std::source_location loc = std::source_location::current()) : Parent(what), _what(what), _loc(loc) {
	}

	const char* what() const noexcept override {
		return _what.c_str();
	}

	std::string location() const override {
		return std::string{_loc.file_name()} + ":" + _loc.function_name() + "@line " + std::to_string(_loc.line()) + ":" + std::to_string(_loc.column());
	}

	virtual ~BaseException() = default;

private:
	std::string          _what;
	std::source_location _loc;
};

inline std::ostream& operator<<(std::ostream& os, const Exception& e) {
	return os << e.what() << " " << e.location();
}

struct RuntimeErrorException : BaseException<std::runtime_error> {
	using BaseException::BaseException;
};

#endif // ME_EXCEPTION_HPP_