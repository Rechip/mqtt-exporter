#pragma once
#ifndef ME_MOSQUITTO_EXCEPTION_HPP_
#	define ME_MOSQUITTO_EXCEPTION_HPP_

#	include "Exception.hpp"

using MosquittoException = BaseException<std::runtime_error>;

#endif // ME_MOSQUITTO_EXCEPTION_HPP_
