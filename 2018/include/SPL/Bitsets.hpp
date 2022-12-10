#pragma once

#include "Utils.hpp"
#include <initializer_list>
#include <type_traits>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wclass-memaccess"

namespace sp{ // BEGINING OF NAMESPACE ///////////////////////////////////////////////////////////







template<size_t cap>
struct BitFiniteArray{
	static_assert(cap, "this makes no sense");

	constexpr size_t unitBits = sizeof(size_t);
	constexpr size_t unitCount = (cap + unitBits-1) / unitBits;

	constexpr size_t shift = sp::logb64(unitBits);
	constexpr size_t mask = (1 << shift) - 1;


	constexpr BitFiniteArray() noexcept : dataSize{} {}
	constexpr BitFiniteArray(const size_t size) noexcept : dataSize{size} {}

	constexpr BitFiniteArray(const BitFiniteArray<cap1> &rhs) noexcept : dataSize{rhs.dataSize} {
		memcpy(data, rhs.data, dataSize >> 3);
	}

	template<size_t cap1>
	constexpr BitFiniteArray &operator =(const FiniteArray<T, cap1> &rhs) noexcept{
		dataSize = rhs.dataSize;
		memcpy(data, rhs.data, dataSize >> 3);
		return *this;
	}


	[[nodiscard]] constexpr bool operator [](const size_t index) const noexcept{
		return (data[index >> shift] >> (index & mask)) & 1;
	}
	constexpr void set(const size_t index) noexcept{
		data[index >> shift] |= 1 << (index & mask);
	}
	constexpr void unset(const size_t index) noexcept{
		data[index >> shift] &= ~(1 << (index & mask));
	}
	constexpr void flip(const size_t index) noexcept{
		data[index >> shift] ^= 1 << (index & mask);
	}
	constexpr void set(const size_t index, const bool x) noexcept{
		data[index >> shift] &= ~(1 << (index & mask));
		data[index >> shift] |= x << (index & mask);
	}


	[[nodiscard]] constexpr bool isFull() const noexcept{ return dataSize >= unitCount*unitBits; }

	[[nodiscard]] constexpr size_t size() const noexcept{ return dataSize; }
	[[nodiscard]] constexpr size_t capacity() const noexcept{ return unitCount * unitBits; }

	[[nodiscard]] constexpr bool front() const noexcept{ return *data & 1; }
	[[nodiscard]] constexpr bool back() const noexcept{
		return (data[dataSize-1 >> shift] >> (dataSize-1 & mask)) & 1;
	}


	constexpr void push_back(const bool arg) noexcept{ data; ++dataSize; }
	constexpr void push_back() noexcept{ ++dataSize; }
	constexpr void pop_back() noexcept{ --dataSize; }

	constexpr void expandBy(const size_t count) noexcept{ dataSize += count; }
	constexpr void skrinkBy(size_t count) noexcept{ dataSize -= count; }

	constexpr void resize(const size_t size) noexcept{ dataSize = size; }


	typedef T value_type;
	union{
		size_t data[unitCount];
	};
	size_t dataSize;
};


template<size_t cap0, size_t cap1>
bool operator !=(
	const BitFiniteArray<cap0> &lhs, const BitFiniteArray<cap1> &rhs
) noexcept{
	return false;
}
template<size_t cap0, size_t cap1>
bool operator ==(
	const BitFiniteArray<cap0> &lhs, const BitFiniteArray<cap1> &rhs
) noexcept{ return !(lhs != rhs); }














template<class T>
struct DArray{
	constexpr DArray() noexcept : beginPtr{nullptr}, endPtr{nullptr}, memEndPtr{nullptr} {}
	constexpr DArray(const size_t size) noexcept :
		beginPtr{(T *)aligned_alloc(alignof(T), size*sizeof(T))},
		endPtr{beginPtr+size}, memEndPtr{endPtr}
	{	
		if constexpr (!std::is_trivially_default_constructible_v<T>)
			for (T *I=beginPtr; I!=endPtr; ++I) new(I) T();
	}
	~DArray() noexcept{
		if constexpr (!std::is_trivially_default_constructible_v<T>)
			for (T *I=beginPtr; I!=endPtr; ++I) I->~T();
		if (beginPtr) free(beginPtr);
	}

	constexpr DArray(const DArray<T> &rhs) noexcept :
		beginPtr{(T *)aligned_alloc(alignof(T), std::size(rhs)*sizeof(T))},
		endPtr{beginPtr+std::size(rhs)}, memEndPtr{endPtr}
	{
		if constexpr (std::is_trivially_copy_constructible_v<T> || std::is_trivial_v<T>){
			memcpy(beginPtr, rhs.beginPtr, std::size(rhs)*sizeof(T));
		} else{
			const T *J = rhs.beginPtr;
			for (T *I=beginPtr; I!=endPtr; ++I, ++J) new(I) T(*J);
		}
	}
	constexpr DArray(DArray<T> &&rhs) noexcept :
		beginPtr{rhs.beginPtr}, endPtr{rhs.endPtr}, memEndPtr{rhs.memEndPtr}
	{
		rhs.beginPtr = rhs.endPtr = rhs.memEndPtr = 0;
	}

	constexpr DArray(const std::initializer_list<T> &rhs) noexcept :
		beginPtr{(T *)aligned_alloc(alignof(T), std::size(rhs)*sizeof(T))},
		endPtr{beginPtr+std::size(rhs)}, memEndPtr{endPtr}
	{
		if constexpr (std::is_trivially_copy_constructible_v<T> || std::is_trivial_v<T>){
			memcpy(beginPtr, rhs.beginPtr, std::size(rhs)*sizeof(T));
		} else{
			const T* I = &*std::begin(rhs); 
			for (T *J=beginPtr; J!=endPtr; ++I, ++J) new(J) T(*I);
		}
	}


	constexpr DArray(const T *first, const T *const last) noexcept :
		beginPtr{(T *)aligned_alloc(alignof(T), (char *)last-(char *)first)},
		endPtr{beginPtr+((char *)last-(char *)first)}, memEndPtr{endPtr}
	{
		if constexpr (std::is_trivially_copy_constructible_v<T> || std::is_trivial_v<T>)
			memcpy(beginPtr, first, (last-first)*sizeof(T));
		else
			for (T *I=beginPtr; first!=last; ++I, ++first) new(I) T(*first);
	}

	template<class It>
	constexpr DArray(It first, const It last) noexcept :
		beginPtr{(T *)aligned_alloc(alignof(T), (char *)last-(char *)first)},
		endPtr{beginPtr+((char *)last-(char *)first)}, memEndPtr{endPtr}
	{
		for (T *I=beginPtr; first!=last; ++I, ++first) new(I) T(*first);
	}

	constexpr DArray &operator =(const DArray<T> &rhs) noexcept{
		const size_t size = std::size(rhs);

		constexpr bool assCond = std::is_trivially_copy_assignable_v<T>;
		constexpr bool conCond = std::is_trivially_copy_constructible_v<T>;
		constexpr bool desCond = std::is_trivially_destructible_v<T>;
		constexpr bool triCond = std::is_trivial_v<T>;

		if (size > endPtr-beginPtr){
			if (size > memEndPtr-beginPtr){
				if constexpr (!desCond)
					for (T *I=beginPtr; I!=endPtr; ++I) I->~T();
				if (beginPtr) free(beginPtr);
				beginPtr = (T *)aligned_alloc(alignof(T), size*sizeof(T));
				memEndPtr = endPtr = beginPtr + size;
				if constexpr (conCond || triCond){
					memcpy(beginPtr, rhs.beginPtr, size*sizeof(T));
				} else{
					const T *J = rhs.beginPtr;
					for (T *I=beginPtr; I!=endPtr; ++I, ++J) new(I) T(*J);
				}
			} else{
				if constexpr ((assCond && conCond) || triCond){
					memcpy(beginPtr, rhs.beginPtr, size*sizeof(T));
					endPtr = beginPtr + size;
				} else{
					if constexpr (assCond){
						memcpy(beginPtr, rhs.beginPtr, (endPtr-beginPtr)*sizeof(T));
						endPtr = beginPtr + size;
					} else{
						T *I = beginPtr;
						for (const T *J=rhs.beginPtr; I!=endPtr; ++I, ++J) *I = *J;
						endPtr = beginPtr + size;
					}
					if constexpr (conCond){
						memcpy(beginPtr, rhs.beginPtr, (endPtr-beginPtr)*sizeof(T));
						endPtr = beginPtr + size;
					} else{
						T *I = endPtr;
						endPtr = beginPtr + size;
						for (const T *J=rhs.beginPtr; I!=endPtr; ++I, ++J) new(I) T(*J);
					}
				}
			}
		} else{
			if constexpr ((assCond && desCond) || triCond){
				memcpy(beginPtr, rhs.beginPtr, size*sizeof(T));
				endPtr = beginPtr + size;
			} else{
				if constexpr (assCond){
					memcpy(beginPtr, rhs.beginPtr, size*sizeof(T));
				} else{
					T *I = beginPtr;
					for (const T *J=rhs.beginPtr; I!=beginPtr+size; ++I, ++J) *I = *J;
				}
				if constexpr (!desCond)
					for (T *I=rhs.beginPtr+size; I!=endPtr; ++I) I->~T();
			}
			endPtr = beginPtr + size;
		}
		return *this;
	}
	constexpr DArray &operator =(DArray<T> &&rhs) noexcept{
		this->~DArray();
		this->beginPtr = rhs.beginPtr;
		this->endPtr = rhs.endPtr;
		this->memEndPtr = rhs.memEndPtr;
		rhs.beginPtr = rhs.endPtr = rhs.memEndPtr = 0;
		return *this;
	}


	constexpr T &operator [](const size_t index) noexcept{ return beginPtr[index]; }
	[[nodiscard]] constexpr const T &operator [](const size_t index) const noexcept{
		return beginPtr[index];
	}

	[[nodiscard]] constexpr bool isFull() const noexcept{ return endPtr == memEndPtr; }

	[[nodiscard]] constexpr size_t size() const noexcept{ return endPtr - beginPtr; }
	[[nodiscard]] constexpr size_t capacity() const noexcept{ return memEndPtr - beginPtr; }


	constexpr T *begin() noexcept{ return beginPtr; }
	constexpr T *end() noexcept{ return endPtr; }
	[[nodiscard]] constexpr const T *begin() const noexcept{ return beginPtr; }
	[[nodiscard]] constexpr const T *end() const noexcept{ return endPtr; }
	[[nodiscard]] constexpr const T *cbegin() const noexcept{ return beginPtr; }
	[[nodiscard]] constexpr const T *cend() const noexcept{ return endPtr; }

	constexpr T &front() noexcept{ return *beginPtr; }
	constexpr T &back() noexcept{ return *(endPtr-1); }
	[[nodiscard]] constexpr const T &front() const noexcept{ return *beginPtr; }
	[[nodiscard]] constexpr const T &back() const noexcept{ return *(endPtr-1); }


	constexpr void push_back(const T &arg) noexcept{
		if (endPtr == memEndPtr){
			const size_t newSize = 2*std::size(*this) + 4*!std::size(*this);
			T *const newBuffer = (T *)aligned_alloc(alignof(T), newSize*sizeof(T));
			if constexpr (std::is_trivially_move_constructible_v<T> || std::is_trivial_v<T>)
				memcpy(newBuffer, beginPtr, (endPtr-beginPtr)*sizeof(T));
			else
				for (T *J=beginPtr, *I=newBuffer; J!=endPtr; ++J, ++I) new(I) T(std::move(*J));
			if (beginPtr) free(beginPtr);
			memEndPtr = newBuffer + newSize;
			endPtr += newBuffer - beginPtr;
			beginPtr = newBuffer;
		}
		new(endPtr) T(arg); ++endPtr;
	}
	constexpr void push_back(T &&arg) noexcept{
		if (endPtr == memEndPtr){
			const size_t newSize = 2*(endPtr-beginPtr) + 4*(endPtr==beginPtr);
			T *const newBuffer = (T *)aligned_alloc(alignof(T), newSize*sizeof(T));
			if constexpr (std::is_trivially_move_constructible_v<T> || std::is_trivial_v<T>)
				memcpy(newBuffer, beginPtr, (endPtr-beginPtr)*sizeof(T));
			else
				for (T *J=beginPtr, *I=newBuffer; J!=endPtr; ++J, ++I) new(I) T(std::move(*J));
			if (beginPtr) free(beginPtr);
			memEndPtr = newBuffer + newSize;
			endPtr += newBuffer - beginPtr;
			beginPtr = newBuffer;
		}
		new(endPtr) T(std::move(arg)); ++endPtr;
	}
	template<class... Args>
	constexpr void emplace_back(Args &&...args) noexcept{
		if (endPtr == memEndPtr){
			const size_t newSize = 2*(endPtr-beginPtr) + 4*(endPtr==beginPtr);
			T *const newBuffer = (T *)aligned_alloc(alignof(T), newSize*sizeof(T));
			if constexpr (std::is_trivially_move_constructible_v<T> || std::is_trivial_v<T>)
				memcpy(newBuffer, beginPtr, (endPtr-beginPtr)*sizeof(T));
			else
				for (T *J=beginPtr, *I=newBuffer; J!=endPtr; ++J, ++I) new(I) T(std::move(*J));
			if (beginPtr) free(beginPtr);
			memEndPtr = newBuffer + newSize;
			endPtr += newBuffer - beginPtr;
			beginPtr = newBuffer;
		}
		new(endPtr) T(std::forward<Args>(args) ...); ++endPtr;
	}
	constexpr void push_back() noexcept{
		if (endPtr == memEndPtr){
			const size_t newSize = 2*std::size(*this) + 4*!std::size(*this);
			T *const newBuffer = (T *)aligned_alloc(alignof(T), newSize*sizeof(T));
			if constexpr (std::is_trivially_move_constructible_v<T> || std::is_trivial_v<T>)
				memcpy(newBuffer, beginPtr, (endPtr-beginPtr)*sizeof(T));
			else
				for (T *J=beginPtr, *I=newBuffer; J!=endPtr; ++J, ++I) new(I) T(std::move(*J));
			if (beginPtr) free(beginPtr);
			memEndPtr = newBuffer + newSize;
			endPtr += newBuffer - beginPtr;
			beginPtr = newBuffer;
		}
		if constexpr (!std::is_trivially_default_constructible_v<T>) new(endPtr) T();
		++endPtr;
	}
	constexpr void pop_back() noexcept{
		--endPtr;
		if constexpr (!std::is_trivially_destructible_v<T>) endPtr->~T();
	}
	constexpr void expandBy(size_t count) noexcept{
		if (endPtr+count > memEndPtr){
			const size_t newSize = (endPtr-beginPtr) + count;
			T *const newBuffer = (T *)aligned_alloc(alignof(T), newSize*sizeof(T));
			if constexpr (std::is_trivially_move_constructible_v<T> || std::is_trivial_v<T>)
				memcpy(newBuffer, beginPtr, (endPtr-beginPtr)*sizeof(T));
			else
				for (T *J=beginPtr, *I=newBuffer; J!=endPtr; ++J, ++I) new(I) T(std::move(*J));
			if (beginPtr) free(beginPtr);
			memEndPtr = newBuffer + newSize;
			endPtr = memEndPtr;
			beginPtr = newBuffer;
		} else endPtr += count;
		if constexpr (!std::is_trivially_default_constructible_v<T>)
			for (T *I=endPtr-count-1; I<endPtr; ++I) new(I) T();
	}
	constexpr void shrinkBy(size_t count) noexcept{
		for (; count; --count){
		  	--endPtr;
			if constexpr (!std::is_trivially_destructible_v<T>) endPtr->~T();
		}
	}
	

	constexpr void resize(const size_t size) noexcept{
		if (size > (size_t)(endPtr-beginPtr)){
			T *const newBuffer = (T *)aligned_alloc(alignof(T), size*sizeof(T));
			if constexpr (std::is_trivially_move_constructible_v<T> || std::is_trivial_v<T>){
				memcpy(newBuffer, beginPtr, (endPtr-beginPtr)*sizeof(T));
			} else{
				T *I = newBuffer;
				for (T *J=beginPtr; J!=endPtr; ++J, ++I) new(I) T(std::move(*J));
			}
			if (beginPtr) free(beginPtr);
			if constexpr (std::is_trivially_default_constructible_v<T> || std::is_trivial_v<T>){
				beginPtr = newBuffer;
				memEndPtr = endPtr = newBuffer + size;
			} else{
				T *I = newBuffer + (endPtr-beginPtr);
				beginPtr = newBuffer;
				memEndPtr = endPtr = newBuffer + size;
				for (; I!=endPtr; ++I) new(I) T();	
			}
		} else{
			T *const newEndPtr = beginPtr + size;
			if constexpr (!std::is_trivially_destructible_v<T>)
				for (T *I=newEndPtr; I!=endPtr; ++I) I->~T();
			endPtr = newEndPtr;
		}
	}

	constexpr void reserve(const size_t size) noexcept{
		if (size > (size_t)(memEndPtr-beginPtr)){
			T *const newBuffer = (T *)aligned_alloc(alignof(T), size*sizeof(T));
			if constexpr (std::is_trivially_move_constructible_v<T> || std::is_trivial_v<T>)
				memcpy(newBuffer, beginPtr, (endPtr-beginPtr)*sizeof(T));
			else
				for (T *J=beginPtr, *I=newBuffer; J!=endPtr; ++J, ++I) new(I) T(std::move(*J));
			if (beginPtr) free(beginPtr);
			memEndPtr = newBuffer + size;
			endPtr += newBuffer - beginPtr;
			beginPtr = newBuffer;
		}
	}


	typedef T value_type;

	T *beginPtr;
	T *endPtr;
	T *memEndPtr;
};


template<class T>
bool operator !=(const sp::DArray<T> &lhs, const sp::DArray<T> &rhs) noexcept{
	if (lhs.endPtr-lhs.beginPtr != rhs.endPtr-lhs.beginPtr) return true;
	for (const T *I=lhs.beginPtr, *J=rhs.beginPtr; I!=lhs.endPtr; ++I, ++J)
		if (*I != *J) return true;
	return false;
}
template<class T>
bool operator ==(
	const sp::DArray<T> &lhs, const sp::DArray<T> &rhs
) noexcept{ return !(lhs != rhs); }










}	// END OF NAMESPACE	///////////////////////////////////////////////////////////////////
