#ifndef ME_VECTOR_ITERATOR_HPP_
#define ME_VECTOR_ITERATOR_HPP_
#pragma once

#include <vector>

template<typename T>
class VectorIterator {
public:
	VectorIterator(std::vector<T> vec, size_t index = 0) : _vec(std::move(vec)), _index(index) {
	}
	VectorIterator(const VectorIterator&) = default;
	VectorIterator(VectorIterator&&)      = default;
	VectorIterator& operator=(const VectorIterator&) = default;
	VectorIterator& operator=(VectorIterator&&) = default;

	VectorIterator& operator++() {
		++_index;
		return *this;
	}

	VectorIterator operator++(int) {
		VectorIterator tmp(*this);
		++(*this);
		return tmp;
	}

	bool operator==(const VectorIterator& other) const {
		return (!isValid() && !other.isValid()) || (_index == other._index && _vec == other._vec);
	}

	bool operator!=(const VectorIterator& other) const {
		return !(*this == other);
	}

	T& operator*() {
		return _vec[_index];
	}

	const T& operator*() const {
		return _vec[_index];
	}

	bool isValid() const {
		return _index < _vec.size();
	}

private:
	std::vector<T> _vec;
	size_t         _index{};
};

#endif
