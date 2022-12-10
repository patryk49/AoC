#pragma once

#include "Utils.hpp"
#include <initializer_list>
#include <type_traits>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wclass-memaccess"

namespace sp{ // BEGINING OF NAMESPACE //////////////////////////////////////////////////////////////////







template<class T>
struct LookUp16Map{
	LookUp16Map() = default;
	LookUp16Map(const LookUp16Map &) = default;
	LookUp16Map(LookUp16Map &&) = default;
	LookUp16Map &operator =(const LookUp16Map &) = default;
	LookUp16Map &operator =(LookUp16Map &&) = default;
	~LookUp16Map() = default;

	constexpr LookUp16Map(const size_t maxId, const size_t size) noexcept{
		keyArray.resize(maxId);
		dataArray.resize(size);
	}

	constexpr T &operator [](const size_t index) noexcept{
		return dataArray[(size_t)keyArray[index]];
	}
	[[nodiscard]] constexpr const T &operator [](const size_t index) const noexcept{
		return dataArray[(size_t)keyArray[index]];
	}

	constexpr void push(const T& data, const size_t index) noexcept{
		keyArray[index] = std::size(dataArray);
		dataArray.push_back(data);
	}

	constexpr void swap(const size_t index1, const size_t index2) noexcept{
		swap((size_t)keyArray.beginPtr+index1, (size_t)keyArray.beginPtr+index2);
	}

	typedef T value_type;

	sp::DArray<uint16_t> keyArray;
	sp::DArray<T> dataArray;
};


}	// END OF NAMESPACE	///////////////////////////////////////////////////////////////////