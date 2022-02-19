#ifndef ME_LOCKABLE_HPP_
#define ME_LOCKABLE_HPP_
#pragma once

#include <mutex>

template<typename T, typename Mutex = std::mutex>
class Lockable : public T, public Mutex {
public:
	using T::T;
};

#endif
