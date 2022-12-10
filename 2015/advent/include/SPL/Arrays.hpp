#pragma once

#include "Utils.hpp"
#include <initializer_list>
#include <type_traits>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wclass-memaccess"

namespace sp{ // BEGINING OF NAMESPACE //////////////////////////////////////////////////////////////////


template<class T>
struct View{
	constexpr T &operator [](const size_t index) noexcept{ return beginPtr[index]; }
	[[nodiscard]] constexpr const T &operator [](const size_t index) const noexcept{ return beginPtr[index]; }


	[[nodiscard]] constexpr size_t size() const noexcept{ return endPtr - beginPtr; }
	constexpr void resize(const size_t size) noexcept{ endPtr = beginPtr + size; }
	
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


	constexpr void push_back(const T &arg) noexcept{ *endPtr = arg; ++endPtr; }
	constexpr void push_back(T &&arg) noexcept{ *endPtr = std::move(arg); ++endPtr; }
	template<class... Args>
	constexpr void emplace_back(Args &&...args) noexcept{
		endPtr->~T();
		new(endPtr) T(std::forward<Args>(args) ...);
		++endPtr;
	}

	constexpr void pop_back() noexcept{ --endPtr; }

	constexpr void expand_back(const size_t count) noexcept{ endPtr += count; }
	constexpr void skrink_back(size_t count) noexcept{ endPtr -= count; }


	constexpr void push_front(const T &arg) noexcept{ *--beginPtr = arg; }
	constexpr void push_front(T &&arg) noexcept{ *--beginPtr = std::move(arg); }
	template<class... Args>
	constexpr void emplace_front(Args &&...args) noexcept{
		--beginPtr;
		beginPtr->~T();
		new(beginPtr) T(std::forward<Args>(args) ...);
	}

	constexpr void pop_front() noexcept{ ++beginPtr; }

	constexpr void expand_front(const size_t count) noexcept{ beginPtr -= count; }
	constexpr void shrink_front(size_t count) noexcept{ beginPtr += count; }


	typedef T value_type;

	T *beginPtr;
	T *endPtr;
};













template<class T, size_t cap>
struct FiniteArray{
	static_assert(cap, "this makes no sense");

	constexpr FiniteArray() noexcept : dataSize{} {}
	constexpr FiniteArray(const size_t size) noexcept : dataSize{size} {
		if constexpr (!std::is_trivially_default_constructible_v<T>)
			for (T *I=data; I!=data+dataSize; ++I) new(I) T();
	}
	~FiniteArray() noexcept{ for (T *I=data; I!=data+dataSize; ++I) I->~T(); }

	template<size_t cap1>
	constexpr FiniteArray(const FiniteArray<T, cap1> &rhs) noexcept : dataSize{std::size(rhs)} {
		const T *J = rhs.data;
		for (T *I=data; I!=data+dataSize; ++I, ++J) new(I) T(*J);
	}
	template<size_t cap1>
	constexpr FiniteArray(FiniteArray<T, cap1> &&rhs) noexcept : dataSize{std::size(rhs)} {
		for (T *I=data, *J=rhs.data; I!=data+dataSize; ++I, ++J) new(I) T(std::move(*J));
		rhs.endPtr = rhs.data;
	}

	constexpr FiniteArray(const std::initializer_list<T> &rhs) noexcept : dataSize{std::size(rhs)} {
		const T *J = &*std::cbegin(rhs);
		for (T *I=data; I!=data+dataSize; ++I, ++J) new(I) T(*J);
	}

	template<class It>
	constexpr FiniteArray(It first, const It last) noexcept : dataSize{last-first} {
		for (T *I=data; I!=data+dataSize; ++I, first) new(I) T(*first);
	}

	template<size_t cap1>
	constexpr FiniteArray &operator =(const FiniteArray<T, cap1> &rhs) noexcept{
		const size_t newSize = std::size(rhs);
		T *I = data;
		if (newSize > dataSize){
			const T *J = std::begin(rhs);
			for (; I!=data+dataSize; ++I, ++J) *I = *J;
			for (; I!=data+newSize; ++I, ++J) new(I) T(*J);
		} else{
			for (const T *J=std::begin(rhs); I!=data+newSize; ++I, ++J) *I = *J;
			for (; I!=data+dataSize; ++I) I->~T();
		}
		dataSize = newSize;
		return *this;
	}
	template<size_t cap1>
	constexpr FiniteArray &operator =(FiniteArray<T, cap1> &&rhs) noexcept{
		const size_t newSize = std::size(rhs);
		T *I = data;
		if (newSize > dataSize){
			const T *J = std::begin(rhs);
			for (; I!=data+dataSize; ++I, ++J) *I = std::move(*J);
			for (; I!=data+newSize; ++I, ++J) new(I) T(std::move(*J));
		} else{
			for (const T *J=std::begin(rhs); I!=data+newSize; ++I, ++J) *I = std::move(*J);
			for (; I!=data+dataSize; ++I) I->~T();
		}
		dataSize = newSize;
		rhs.endPtr = rhs.data;
		return *this;
	}


	constexpr T &operator [](const size_t index) noexcept{ return data[index]; }
	[[nodiscard]] constexpr const T &operator [](const size_t index) const noexcept{ return data[index]; }


	[[nodiscard]] constexpr bool isFull() const noexcept{ return dataSize >= cap; }

	[[nodiscard]] constexpr size_t size() const noexcept{ return dataSize; }
	[[nodiscard]] constexpr size_t capacity() const noexcept{ return cap; }

	constexpr T *begin() noexcept{ return data; }
	constexpr T *end() noexcept{ return data + dataSize; }
	[[nodiscard]] constexpr const T *begin() const noexcept{ return data; }
	[[nodiscard]] constexpr const T *end() const noexcept{ return data + dataSize; }
	[[nodiscard]] constexpr const T *cbegin() const noexcept{ return data; }
	[[nodiscard]] constexpr const T *cend() const noexcept{ return data + dataSize; }

	constexpr T &front() noexcept{ return *data; }
	constexpr T &back() noexcept{ return *(data + dataSize - 1); }
	[[nodiscard]] constexpr const T &front() const noexcept{ return *data; }
	[[nodiscard]] constexpr const T &back() const noexcept{ return *(data + dataSize - 1); }


	constexpr void push_back(const T &arg) noexcept{ new(data+dataSize) T(arg); ++dataSize; }

	constexpr void push_back(T &&arg) noexcept{ new(data+dataSize) T(std::move(arg)); ++dataSize; }

	template<class... Args>
	constexpr void emplace_back(Args &&...args) noexcept{ new(data+dataSize) T(std::forward<Args>(args) ...); ++dataSize; }

	constexpr void pop_back() noexcept{ (data+--dataSize)->~T(); }

	constexpr void expandBy(const size_t count) noexcept{
		T *I = data + dataSize;
		dataSize += count;
		if constexpr (!std::is_trivially_default_constructible_v<T>)
			for (; I!=data+dataSize; ++I) new(I) T();
	}
	constexpr void skrinkBy(size_t count) noexcept{ for (; count; --count) (data+--dataSize)->~T(); }


	constexpr void resize(const size_t size) noexcept{
		if (size > dataSize){
			if constexpr (!std::is_trivially_default_constructible_v<T>)
				for (T *I=data+dataSize; I!=data+size; ++I) new(I) T();
		} else
			for (T *I=data+size; I!=data+dataSize; ++I) I->~T();

		dataSize = size;
	}


	typedef T value_type;

private:
	union{
		T data[cap];
	};
	size_t dataSize;
};















template<class T>
struct DArray{
	constexpr DArray() noexcept : beginPtr{nullptr}, endPtr{nullptr}, memEndPtr{nullptr} {}
	constexpr DArray(const size_t size) noexcept :
		beginPtr{(T *)aligned_alloc(alignof(T), size*sizeof(T))}, endPtr{beginPtr+size}, memEndPtr{endPtr}
	{	
		if constexpr (!std::is_trivially_default_constructible_v<T>)
			for (T *I=beginPtr; I!=endPtr; ++I) new(I) T();
	}
	~DArray() noexcept{
		for (T *I=beginPtr; I!=endPtr; ++I) I->~T();
		if (beginPtr) free(beginPtr);
	}

	constexpr DArray(const DArray<T> &rhs) noexcept :
		beginPtr{(T *)aligned_alloc(alignof(T), std::size(rhs)*sizeof(T))}, endPtr{beginPtr+std::size(rhs)}, memEndPtr{endPtr}
	{
		const T *J = rhs.beginPtr;
		for (T *I=beginPtr; I!=endPtr; ++I, ++J) new(I) T(*J);
	}
	constexpr DArray(DArray<T> &&rhs) noexcept : beginPtr{rhs.beginPtr}, endPtr{rhs.endPtr}, memEndPtr{rhs.memEndPtr} {
		rhs.beginPtr = rhs.endPtr = rhs.memEndPtr = 0;
	}

	constexpr DArray(const std::initializer_list<T> &rhs) noexcept :
		beginPtr{(T *)aligned_alloc(alignof(T), std::size(rhs)*sizeof(T))},
		endPtr{beginPtr+std::size(rhs)}, memEndPtr{endPtr}
	{
		const T* I = &*std::begin(rhs); 
		for (T *J=beginPtr; J!=endPtr; ++I, ++J) new(J) T(*I);
	}

	template<class It>
	constexpr DArray(It first, const It last) noexcept :
		beginPtr{(T *)aligned_alloc(alignof(T), (char *)last-(char *)first)},
		endPtr{beginPtr+((char *)last-(char *)first)}, memEndPtr{endPtr}
	{
		for (T *I=beginPtr; I!=endPtr; ++I, ++first) new(I) T(*first);
	}

	constexpr DArray &operator =(const DArray<T> &rhs) noexcept{
		const size_t size = std::size(rhs);
		if (size > std::size(*this)){
			if (size > memEndPtr-beginPtr){
				for (T *I=beginPtr; I!=endPtr; ++I) I->~T();
				if (beginPtr) free(beginPtr);
				beginPtr = (T *)aligned_alloc(alignof(T), size*sizeof(T));
				memEndPtr = endPtr = beginPtr + size;
				const T *J = std::begin(rhs);
				for (T *I=beginPtr; I!=endPtr; ++I, ++J) new(I) T(*J);
			} else{
				T *I = beginPtr;
				const T *J = std::begin(rhs);
				for (; I!=endPtr; ++I, ++J) *I = *J;
				endPtr = beginPtr + size;
				for (; I!=endPtr; ++I, ++J) new(I) T(*J);
			}
		} else{
			T *const newEndPtr = beginPtr + size;
			T *I = beginPtr;
			const T *J = std::begin(rhs);
			for (; I!=newEndPtr; ++I, ++J) *I = *J;
			for (; I!=endPtr; ++I) I->~T();
			endPtr = newEndPtr;
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
	[[nodiscard]] constexpr const T &operator [](const size_t index) const noexcept{ return beginPtr[index]; }

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
			const size_t newSize = 2*std::size(*this) + 4*!std::size(*this);
			T *const newBuffer = (T *)aligned_alloc(alignof(T), newSize*sizeof(T));
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
			const size_t newSize = 2*std::size(*this) + 4*!std::size(*this);
			T *const newBuffer = (T *)aligned_alloc(alignof(T), newSize*sizeof(T));
			for (T *J=beginPtr, *I=newBuffer; J!=endPtr; ++J, ++I)
				new(I) T(std::move(*J));
			if (beginPtr) free(beginPtr);
			memEndPtr = newBuffer + newSize;
			endPtr += newBuffer - beginPtr;
			beginPtr = newBuffer;
		}
		new(endPtr) T(std::forward<Args>(args) ...); ++endPtr;
	}
	constexpr void pop_back() noexcept{ (--endPtr)->~T(); }
	constexpr void expandBy(size_t count) noexcept{
		if (endPtr+count > memEndPtr){
			const size_t newSize = std::size(*this) + count;
			T *const newBuffer = (T *)aligned_alloc(alignof(T), newSize*sizeof(T));
			for (T *J=beginPtr, *I=newBuffer; J!=endPtr; ++J, ++I)
				new(I) T(std::move(*J));
			if (beginPtr) free(beginPtr);
			memEndPtr = newBuffer + newSize;
			endPtr = memEndPtr;
			beginPtr = newBuffer;
		} else endPtr += count;
		if constexpr (!std::is_trivially_default_constructible_v<T>)
			for (T *I=endPtr-count-1; I<endPtr; ++I) new(I) T();
	}
	constexpr void shrinkBy(size_t count) noexcept{ for (; count; --count) (--endPtr)->~T(); }
	

	constexpr void resize(const size_t size) noexcept{
		if (size > std::size(*this)){
			T *const newBuffer = (T *)aligned_alloc(alignof(T), size*sizeof(T));
			T *I = newBuffer;
			for (T *J=beginPtr; J!=endPtr; ++J, ++I) new(I) T(std::move(*J));
			if (beginPtr) free(beginPtr);
			beginPtr = newBuffer;
			memEndPtr = endPtr = newBuffer + size;
			if constexpr (!std::is_trivially_default_constructible_v<T>)
				for (; I!=endPtr; ++I) new(I) T();
		} else{
			T *const newEndPtr = beginPtr + size;
			for (T *I=newEndPtr; I!=endPtr; ++I) I->~T();
			endPtr = newEndPtr;
		}
	}

	constexpr void reserve(const size_t size) noexcept{
		if (size > (size_t)(memEndPtr-beginPtr)){
			T *const newBuffer = (T *)aligned_alloc(alignof(T), size*sizeof(T));
			for (T *J=beginPtr, *I=newBuffer; J!=endPtr; ++J, ++I) new(I) T(std::move(*J));
			if (beginPtr) free(beginPtr);
			memEndPtr = newBuffer + size;
			endPtr += newBuffer - beginPtr;
			beginPtr = newBuffer;
		}
	}


	typedef T value_type;

private:
	T *beginPtr;
	T *endPtr;
	T *memEndPtr;
};















template<class T, size_t buffSize = (sizeof(T)+15) / sizeof(T)>
struct SBArray{
	static_assert(buffSize, "this makes no sense");

// if the most significant bit of dataSize is 1, the the data is separately allocated
	static constexpr size_t allocFlagMask = (size_t)1 << (sizeof(size_t)*8-1);
	static constexpr size_t sizeMask = ~allocFlagMask;

	constexpr SBArray() noexcept : dataSize{} {}

	constexpr SBArray(const size_t size) noexcept{
		if (size > buffSize){
			dataPtr = (T *)aligned_alloc(alignof(T), size*sizeof(T));
			dynCap = size;
			dataSize = size | allocFlagMask;
			if constexpr (!std::is_trivially_default_constructible_v<T>)
				for (T *I=dataPtr; I!=dataPtr+size; ++I) new(I) T();
		} else{
			dataSize = -size;
			if constexpr (!std::is_trivially_default_constructible_v<T>)
				for (T *I=buffer; I!=buffer+size; ++I) new(I) T();
		}
	}

	template<size_t buffSize1>
	constexpr SBArray(const SBArray<T, buffSize1> &rhs) noexcept{
		const size_t size = std::size(rhs);
		if (size > buffSize){
			dataPtr = (T *)aligned_alloc(alignof(T), size*sizeof(T));
			dynCap = size;
			dataSize = size | allocFlagMask;
			const T *J = std::begin(rhs);
			for (T *I=dataPtr; I!=dataPtr+size; ++I, ++J) new(I) T(*J);
		} else{
			dataSize = size;
			const T *J = std::begin(rhs);
			for (T *I=buffer; I!=buffer+size; ++I, ++J) new(I) T(*J);
		}
	}

	template<size_t buffSize1>
	constexpr SBArray(SBArray<T, buffSize1> &&rhs) noexcept {
		if (rhs.usesBuffer()){
			const size_t size = rhs.dataSize & sizeMask;
			dataSize = size;
			if constexpr (buffSize1 >= buffSize)
				if (size > buffSize){
					dataPtr = (T *)aligned_alloc(alignof(T), size*sizeof(T));
					dataSize |= allocFlagMask;
					dynCap = size;
					T *J = std::begin(rhs);
					for (T *I=dataPtr; I!=dataPtr+size; ++I, ++J) new(I) T(std::move(*J));
				}
			T *J = std::begin(rhs);
			for (T *I=std::begin(*this); I!=std::end(*this); ++I, ++J) new(I) T(std::move(*J));
		} else{
			dataSize = rhs.dataSize;
			dataPtr = rhs.dataPtr;
			dynCap = rhs.dynCap;
		}
		rhs.dataSize = 0;
	}

	constexpr SBArray(const std::initializer_list<T> &rhs) noexcept{
		const size_t size = std::size(rhs);
		if (size > buffSize){
			dataPtr = (T *)aligned_alloc(alignof(T), size*sizeof(T));
			dynCap = size;
			dataSize = size | allocFlagMask;
			const T *J = std::begin(rhs);
			for (T *I=dataPtr; I!=dataPtr+size; ++I, ++J) new(I) T(*J);
		} else{
			dataSize = size;
			const T *J = std::begin(rhs);
			for (T *I=buffer; I!=buffer+size; ++I, ++J) new(I) T(*J);
		}
	}

	template<class It>
	constexpr SBArray(It first, const It last) noexcept{
		const size_t size = last - first;
		if (size > buffSize){
			dataPtr = (T *)aligned_alloc(alignof(T), size*sizeof(T));
			dynCap = size;
			dataSize = size | allocFlagMask;
			for (T *I=dataPtr; first!=last; ++I, ++first) new(I) T(*first);
		} else{
			dataSize = size;
			for (T *I=buffer; first!=last; ++I, ++first) new(I) T(*first);
		}
	}

	template<size_t buffSize1>
	constexpr SBArray &operator =(const SBArray<T, buffSize1> &rhs) noexcept{
		const size_t size = std::size(rhs);
		if (this->usesBuffer()){
			if (size > buffSize){
				for (T *I; I!=buffer+dataSize; ++I) I->~T();
				dataSize = size | allocFlagMask;
				dataPtr = (T *)aligned_alloc(alignof(T), size*sizeof(T));
				dynCap = size;
				const T *J = std::begin(rhs);
				for (T *I=dataPtr; I!=dataPtr+size; ++I, ++J) new(I) T(*J);
			} else{
				T *I = buffer;
				if (size > dataSize){
					const T *J = std::begin(rhs);
					for (; I!=buffer+dataSize; ++I, ++J) *I = *J;
					for (; I!=buffer+size; ++I, ++J) new(I) T(*J);
				} else{
					for (const T *J=std::begin(rhs); I!=buffer+size; ++I, ++J) *I = *J;
					for (; I!=buffer+dataSize; ++I) I->~T();
				}
				dataSize = size;
			}
		} else{
			if (size > (dataSize&sizeMask)){
				if (size > dynCap){
					for (T *I=dataPtr; I!=dataPtr+(dataSize&sizeMask); ++I) I->~T();
					if (dataPtr) free(dataPtr);
					dataPtr = (T *)aligned_alloc(alignof(T), size*sizeof(T));
					dynCap = size;
					const T *J = std::begin(rhs);
					for (T *I=dataPtr; I!=dataPtr+size; ++I, ++J) new(I) T(*J);
				} else{
					T *I = dataPtr;
					const T *J = std::begin(rhs);
					for (; I!=dataPtr+(dataSize&sizeMask); ++I, ++J) *I = *J;
					for (; I!=dataPtr+size; ++I, ++J) new(I) T(*J);
				}
			} else{
				T *I = dataPtr;
				const T *J = std::begin(rhs);
				for (; I!=dataPtr+size; ++I, ++J) *I = *J;
				for (; I!=dataPtr+(dataSize&sizeMask); ++I) I->~T();
			}
			dataSize = size | allocFlagMask;
		}
		return *this;
	}
	
	template<size_t buffSize1>
	constexpr SBArray &operator =(SBArray<T, buffSize1> &&rhs) noexcept{
		if (rhs.usesBuffer()){
			const size_t size = std::size(rhs);
			if (this->usesBuffer()){
				if constexpr (buffSize1 > buffSize){
					if (size > buffSize){
						for (T *I; I!=buffer+dataSize; ++I) I->~T();
						dataSize = size | allocFlagMask;
						dataPtr = (T *)aligned_alloc(alignof(T), size*sizeof(T));
						dynCap = size;
						T *J = std::begin(rhs);
						for (T *I=dataPtr; I!=dataPtr+size; ++I, ++J) new(I) T(std::move(*J));
					} else{
						T *I = buffer;
						if (size > dataSize){
							const T *J = std::begin(rhs);
							for (; I!=buffer+dataSize; ++I, ++J) *I = std::move(*J);
							for (; I!=buffer+size; ++I, ++J) new(I) T(std::move(*J));
						} else{
							for (const T *J=std::begin(rhs); I!=buffer+size; ++I, ++J) *I = std::move(*J);
							for (; I!=buffer+dataSize; ++I) I->~T();
						}
						dataSize = size;
					}
				} else{
					T *I = buffer;
					if (size > dataSize){
						T *J = std::begin(rhs);
						for (; I!=buffer+dataSize; ++I, ++J) *I = std::move(*J);
						for (; I!=buffer+size; ++I, ++J) new(I) T(std::move(*J));
					} else{
						for (const T *J=std::begin(rhs); I!=buffer+size; ++I, ++J) *I = std::move(*J);
						for (; I!=buffer+dataSize; ++I) I->~T();
					}
					dataSize = size;
				}
			} else{
				if (size > (dataSize&sizeMask)){
					if (size > dynCap){
						for (T *I=dataPtr; I!=dataPtr+(dataSize&sizeMask); ++I) I->~T();
						if (dataPtr) free(dataPtr);
						dataPtr = (T *)aligned_alloc(alignof(T), size*sizeof(T));
						dynCap = size;
						T *J = std::begin(rhs);
						for (T *I=dataPtr; I!=dataPtr+size; ++I, ++J) new(I) T(std::move(*J));
					} else{
						T *I = dataPtr;
						const T *J = std::begin(rhs);
						for (; I!=dataPtr+(dataSize&sizeMask); ++I, ++J) *I = std::move(*J);
						for (; I!=dataPtr+size; ++I, ++J) new(I) T(std::move(*J));
					}
				} else{
					T *I = dataPtr;
					T *J = std::begin(rhs);
					for (; I!=dataPtr+size; ++I, ++J) *I = std::move(*J);
					for (; I!=dataPtr+(dataPtr&sizeMask); ++I) I->~T();
				}
				dataSize = size | allocFlagMask;
			}
		} else{
			this->~SBArray();
			dataSize = rhs.dataSize;
			dataPtr = rhs.dataPtr;
			dynCap = rhs.dynCap;
		}
		rhs.dataSize = 0;
		return *this;
	}

	~SBArray() noexcept{
		for (T *I=std::begin(*this); I!=std::end(*this); ++I) I->~T();
		if (!this->usesBuffer()) free(dataPtr);
	}


	constexpr T &operator [](const size_t index) noexcept{ return (usesBuffer() ? buffer : dataPtr)[index]; }
	[[nodiscard]] constexpr const T &operator [](const size_t index) const noexcept{ return (usesBuffer() ? buffer : dataPtr)[index]; }


	[[nodiscard]] constexpr bool usesBuffer() const noexcept{ return !(dataSize & allocFlagMask) ; }

	[[nodiscard]] constexpr size_t size() const noexcept{ return dataSize & sizeMask; }
	[[nodiscard]] constexpr size_t capacity() const noexcept{ return dataSize&allocFlagMask ? dynCap : buffSize; }

	constexpr T *begin() noexcept{ return usesBuffer() ? buffer : dataPtr; }
	constexpr T *end() noexcept{ return (usesBuffer() ? buffer : dataPtr) + (dataSize & sizeMask); }
	[[nodiscard]] constexpr const T *begin() const noexcept{ return usesBuffer() ? buffer : dataPtr; }
	[[nodiscard]] constexpr const T *end() const noexcept{ return (usesBuffer() ? buffer : dataPtr) + (dataSize & sizeMask); }
	[[nodiscard]] constexpr const T *cbegin() const noexcept{ return usesBuffer() ? buffer : dataPtr; }
	[[nodiscard]] constexpr const T *cend() const noexcept{ return (usesBuffer() ? buffer : dataPtr) + (dataSize & sizeMask); }

	constexpr T &front() noexcept{ return *begin(); }
	constexpr T &back() noexcept{ return *(end()-1); }
	[[nodiscard]] constexpr const T &front() const noexcept{ return *begin(); }
	[[nodiscard]] constexpr const T &back() const noexcept{ return *(end()-1); }


	constexpr void push_back(const T &arg) noexcept{
		if (usesBuffer()){
			if (dataSize+1 > buffSize){
				T *const newBuffer = (T *)aligned_alloc(alignof(T), (dataSize+buffSize)*sizeof(T));
				T *I = newBuffer;
				for (T *J=buffer; J!=buffer+(dataSize&sizeMask); ++I, ++J) new(I) T(std::move(*J));
				new(I) T(arg);
				dataPtr = newBuffer;
				dynCap = dataSize + buffSize;
				dataSize = (dataSize + 1) | allocFlagMask;
			} else{
				new(buffer + dataSize) T(arg);
				++dataSize;
			}
		} else{
			if ((dataSize&sizeMask)+1 > dynCap){
				T *const newBuffer = (T *)aligned_alloc(alignof(T), (2*(dataSize&sizeMask))*sizeof(T));
				T *I = newBuffer;
				for (T *J=dataPtr; J!=dataPtr+(dataSize&sizeMask); ++I, ++J) new(I) T(std::move(*J));
				new(I) T(arg);
				dynCap = 2 * (dataSize&sizeMask);
				dataPtr = newBuffer;
			} else{
				new(dataPtr + (dataSize&sizeMask)) T(arg);
			}
			++dataSize;
		}
	}

	constexpr void push_back(T &&arg) noexcept{
		if (usesBuffer()){
			if (dataSize+1 > buffSize){
				T *const newBuffer = (T *)aligned_alloc(alignof(T), (dataSize+buffSize)*sizeof(T));
				T *I = newBuffer;
				for (T *J=buffer; J!=buffer+(dataSize&sizeMask); ++I, ++J) new(I) T(std::move(*J));
				new(I) T(std::move(arg));
				dataPtr = newBuffer;
				dynCap = dataSize + buffSize;
				dataSize = (dataSize + 1) | allocFlagMask;
			} else{
				new(buffer + dataSize) T(std::move(arg));
				++dataSize;
			}
		} else{
			if ((dataSize&sizeMask)+1 > dynCap){
				T *const newBuffer = (T *)aligned_alloc(alignof(T), (2*(dataSize&sizeMask))*sizeof(T));
				T *I = newBuffer;
				for (T *J=dataPtr; J!=dataPtr+(dataSize&sizeMask); ++I, ++J) new(I) T(std::move(*J));
				new(I) T(std::move(arg));
				dynCap = 2 * (dataSize&sizeMask);
				dataPtr = newBuffer;
			} else{
				new(dataPtr + (dataSize&sizeMask)) T(std::move(arg));
			}
			++dataSize;
		}
	}

	template<class... Args>
	constexpr void emplace_back(Args &&...arg) noexcept{
		if (usesBuffer()){
			if (dataSize+1 > buffSize){
				T *const newBuffer = (T *)aligned_alloc(alignof(T), (dataSize+buffSize)*sizeof(T));
				T *I = newBuffer;
				for (T *J=buffer; J!=buffer+(dataSize&sizeMask); ++I, ++J) new(I) T(std::move(*J));
				new(I) T(std::forward<Args>(arg) ...);
				dataPtr = newBuffer;
				dynCap = dataSize + buffSize;
				dataSize = (dataSize + 1) | allocFlagMask;
			} else{
				new(buffer + dataSize) T(std::forward<Args>(arg) ...);
				++dataSize;
			}
		} else{
			if ((dataSize&sizeMask)+1 > dynCap){
				T *const newBuffer = (T *)aligned_alloc(alignof(T), (2*(dataSize&sizeMask))*sizeof(T));
				T *I = newBuffer;
				for (T *J=dataPtr; J!=dataPtr+(dataSize&sizeMask); ++I, ++J) new(I) T(std::move(*J));
				new(I) T(std::forward<Args>(arg) ...);
				dynCap = 2 * (dataSize&sizeMask);
				dataPtr = newBuffer;
			} else{
				new(dataPtr + (dataSize&sizeMask)) T(std::forward<Args>(arg) ...);
			}
			++dataSize;
		}
	}
	constexpr void pop_back() noexcept{
		--dataSize;
		((this->usesBuffer() ? buffer : dataPtr) + (dataSize&sizeMask))->~T();
	}
	constexpr void expandBy(const size_t count) noexcept{ this->resize((dataSize&sizeMask) - count); }
	constexpr void shrinkBy(size_t count) noexcept{
		T *const endPtr = (this->usesBuffer() ? buffer : dataPtr) + (dataSize&sizeMask); 
		dataSize -= count;
		for (T *I=endPtr-count; I!=endPtr; ++I) I->~T();
	}


	constexpr void resize(const size_t size) noexcept{
		if (size > (dataSize&sizeMask)){
			if (usesBuffer()){
				if (size > buffSize){
					T *const newBuffer = (T *)aligned_alloc(alignof(T), size*sizeof(T));
					T *I = newBuffer;
					for (T *J=buffer; J!=buffer+(dataSize&sizeMask); ++I, ++J) new(I) T(std::move(*J));
					if constexpr (!std::is_trivially_default_constructible_v<T>)
						for (; I!=newBuffer+size; ++I) new(I) T();
					dataSize = size | allocFlagMask;
					dataPtr = newBuffer;
					dynCap = size;
				} else{
					if constexpr (!std::is_trivially_default_constructible_v<T>)
						for (T *I=buffer+(dataSize&sizeMask); I!=buffer+size; ++I) new(I) T();
					dataSize = size;
				}
			} else{
				if ((dataSize&sizeMask) > dynCap){
					T *const newBuffer = (T *)aligned_alloc(alignof(T), size*sizeof(T));
					T *I = newBuffer;
					for (T *J=dataPtr; J!=dataPtr+(dataSize&sizeMask); ++I, ++J) new(I) T(std::move(*J));
					if constexpr (!std::is_trivially_default_constructible_v<T>)
						for (; I!=newBuffer+size; ++I) new(I) T();
					free(dataPtr);
					dataPtr = newBuffer;
					dynCap = size;
				} else{
					if constexpr (!std::is_trivially_default_constructible_v<T>)
						for (T *I=dataPtr+(dataSize&sizeMask); I!=buffer+size; ++I) new(I) T();
				}
				dataSize = size | allocFlagMask;
			}
		} else{
			for (T *I=begin()+size; I!=end(); ++I) I->~T();
			dataSize &= allocFlagMask;
			dataSize |= size;
		}
	}

	constexpr void reserve(const size_t size) noexcept{
		if (usesBuffer()){
			if (size > buffSize){
				T *const newBuffer = (T *)aligned_alloc(alignof(T), size*sizeof(T));
				for (T *I=newBuffer, *J=buffer;  J!=buffer+(dataSize&sizeMask); ++I, ++J) new(I) T(std::move(*J));
				dataSize |= allocFlagMask;
				dataPtr = newBuffer;
				dynCap = size;
			}
		} else{
			if (size > dynCap){
				T *const newBuffer = (T *)aligned_alloc(alignof(T), size*sizeof(T));
				for (T *I=newBuffer, *J=dataPtr;  J!=dataPtr+(dataSize&sizeMask); ++I, ++J) new(I) T(std::move(*J));
				free(dataPtr);
				dataPtr = newBuffer;
				dynCap = size;
			}
		}
	}


	typedef T value_type;

private:
	size_t dataSize;
	union{
		struct{
			T *dataPtr;
			size_t dynCap;
		};
		T buffer[buffSize];
	};
};













template<class T>
struct PtrArray{
	PtrArray() noexcept : ptr{nullptr} {}
	PtrArray(const PtrArray &x) noexcept{
		if (x.ptr){
			uint32_t *const memBegin = (uint32_t *)aligned_alloc(std::max(alignof(T), alignof(uint32_t)),
				std::size(x)*sizeof(T) + 2*sizeof(uint32_t)
			);
			memBegin[0] = x.capacity();
			memBegin[1] = std::size(x);
			ptr = (T *)(memBegin + 2);

			const T *const endPtr = ptr + std::size(x);
			const T *J = x.ptr;
			for (T *I=ptr; I!=endPtr; ++I, ++J) new(I) T(*J);
		} else ptr = nullptr;
	}
	PtrArray(PtrArray &&x) noexcept{
		ptr = x.ptr;
		x.ptr = nullptr; 
	}
	PtrArray &operator =(const PtrArray &x) noexcept{
		if (this->size() < std::size(x)){
			if (ptr) free((uint32_t*)ptr-2);
			uint32_t *const memBegin = (uint32_t *)aligned_alloc(std::max(alignof(T), alignof(uint32_t)),
				std::size(x)*sizeof(T) + 2*sizeof(uint32_t)
			);
			memBegin[0] = x.capacity();
			memBegin[1] = std::size(x);
			ptr = (T *)(memBegin + 2);
			
			const T *const endPtr = ptr + std::size(x);
			const T *J = x.ptr;
			for (T *I=ptr; I!=endPtr; ++I, ++J) new(I) T(*J);
		} else{
			const T *const newEndPtr = ptr + std::size(x);
			T *I = ptr;
			for (const T *J=x.ptr; I!=newEndPtr; ++I, ++J) *I = *J;
			T *endPtr = ptr + *((uint32_t *)ptr - 1); 
			for (; I!=endPtr; ++I) I->~T();

			*((uint32_t *)ptr - 1) = std::size(x);
		}
	}
	PtrArray &operator =(PtrArray &&x) noexcept{
		this->~PtrArray();
		ptr = x.ptr;
		x.ptr = nullptr; 
	}
	~PtrArray(){
		if (ptr){	
			const T *const endPtr = *((uint32_t *)ptr-1);
			for (T *I=ptr; I!=endPtr; ++I) I.~T();
			free((uint32_t *)ptr-2);
		}
	}

	[[nodiscard]] size_t size() const noexcept{ return ptr ? *((uint32_t *)ptr - 1) : 0; }
	[[nodiscard]] size_t capacity() const noexcept{ return ptr ? *((uint32_t *)ptr - 2) : 0; }
	[[nodiscard]] const T *begin() const noexcept{ return ptr; }
	[[nodiscard]] const T *end() const noexcept{ return ptr + this->size(); }
	T *begin() noexcept{ return ptr; }
	T *end() noexcept{ return ptr + this->size(); }
	[[nodiscard]] const T *cbegin() const noexcept{ return ptr; }
	[[nodiscard]] const T *cend() const noexcept{return ptr + this->size(); }
	T &front() noexcept{ return *ptr; }
	T &back() noexcept{ return ptr[this->size() - 1]; }
	[[nodiscard]] const T &front() const noexcept{ return *ptr; }
	[[nodiscard]] const T &back() const noexcept{ return ptr[this->size() - 1]; }

	void resize(const uint32_t newSize){
		if (this->capacity() < newSize){
			uint32_t *const memBegin = (uint32_t *)aligned_alloc(std::max(alignof(T), alignof(uint32_t)),
				newSize*sizeof(T) + 2*sizeof(uint32_t)
			);
			memBegin[0] = newSize;
			memBegin[1] = newSize;
			/* MOVING LOOP */{
				T *I = (T *)(memBegin + 2);
				const T *const endPtr = ptr + *((uint32_t *)ptr - 1);
				for (T *J=ptr; J!=endPtr; ++I, ++J) new(I) T(std::move(J));
			}
			if (ptr) free((uint32_t*)ptr-2);
			ptr = (T *)(memBegin + 2);
		} else{
			for (T *I=this->begin()+newSize; I!=this->end(); ++I) I->~T();
			*((uint32_t*)ptr - 1) = newSize;
		}
	}
	void reserve(const uint32_t newCapacity){
		if (this->capacity() < newCapacity){
			uint32_t *const memBegin = (uint32_t *)aligned_alloc(std::max(alignof(T), alignof(uint32_t)),
				newCapacity*sizeof(T) + 2*sizeof(uint32_t)
			);
			memBegin[0] = newCapacity;
			memBegin[1] = this->size();
			/* MOVING LOOP */{
				T *I = (T *)(memBegin + 2);
				const T *const endPtr = ptr + *((uint32_t *)ptr - 1);
				for (T *J=ptr; J!=endPtr; ++I, ++J) new(I) T(std::move(J));
			}
			if (ptr) free((uint32_t*)ptr-2);
			ptr = (T *)(memBegin + 2);
		}
	}
	void push_back(const T &x){
		if (this->size() >= this->capacity())
			this->reserve(this->size()*2);
		new(ptr + this->size()) T(x);
		++*((uint32_t *)ptr - 1);
	}
	void push_back(T &&x){
		if (this->size() >= this->capacity())
			this->reserve(this->size()*2);
		new(ptr + this->size()) T(std::move(x));
		++*((uint32_t *)ptr - 1);
	}
	template<class ...Args>
	void emplace_back(Args &&...args){
		if (this->size() >= this->capacity())
			this->reserve(this->size()*2);
		new(ptr + this->size()) T(std::forward<Args>(args) ...);
		++*((uint32_t *)ptr - 1);
	}

	void expandBy(const size_t len) noexcept{ this->resize(this->size + len); }
	void shrinkBy(const size_t len) noexcept{
		const T *const endPtr = ptr + *((uint32_t*)ptr - 1);
		for (T *I=endPtr-len; I!=endPtr; ++I) I->~T();
		*((uint32_t*)ptr - 1) = (endPtr - ptr) - len;
	}


private:
	T *ptr;
};












// template<class T, size_t blockSize_ = 64, size_t maxBlockNumber_ = 16>
// struct StableArray{
// 	constexpr StableArray() noexcept : size_{0} {}

// 	constexpr StableArray(const size_t newSize) noexcept : size_{newSize}{
// 		for (size_t i=0; i<newSize; i+=blockSize_)
// 			ptrTable[i] = new T[blockSize];
// 	}

// 	template<size_t blksize, size_t blknum>
// 	constexpr StableArray(const StableArray<T, blksize, blknum> &rhs) noexcept : size_{std::size(rhs)}{
// 		size_t i = size_;
// 		size_t j = 0;
// 		size_t k = 0;
// 		for (; i>=blockSize_; i-=blockSize_, ++j){
// 			ptrTable[j] = (T *) aligned_alloc(alignof(T), blockSize_*sizeof(T));
// 			const T *const end = ptrTable[j] + blockSize_;
// 			const T *J = std::begin(rhs) + j*blockSize_;
// 			for (T *I = ptrTable[j]; I!=end; ++I, ++k)
// 				new(I) T{rhs[k]};
// 		}
// 		if (i){
// 			ptrTable[j] = (T *) aligned_alloc(alignof(T), blockSize_*sizeof(T));
// 			const T *const end = ptrTable[j] + i;
// 			const T *J = std::begin(rhs) + j*blockSize_;
// 			for (T *I = ptrTable[j]; I!=end; ++I, ++k)
// 				new(I) T{rhs[k]};
// 		}
// 	}

// 	template<size_t blknum>
// 	constexpr StableArray(StableArray<T, blockSize_, blknum> &&rhs) noexcept : size_{std::size(rhs)}{
// 		std::copy_n(rhs.ptrTable, rhs.filledBlockNumber(), ptrTable);
// 		rhs.size = 0;
// 	}

// 	constexpr StableArray(const std::initializer_list<T> &rhs) noexcept : size_{std::size(rhs)}{
// 		size_t i = size_;
// 		size_t j = 0;
// 		for (; i>=blockSize_; i-=blockSize_, ++j){
// 			ptrTable[j] = (T *) aligned_alloc(alignof(T), blockSize_*sizeof(T));
// 			const T *const end = ptrTable[j] + blockSize_;
// 			const T *J = std::begin(rhs) + j*blockSize_;
// 			for (T *I = ptrTable[j]; I!=end; ++I, ++J)
// 				new(I) T{*J};
// 		}
// 		if (i){
// 			ptrTable[j] = (T *) aligned_alloc(alignof(T), blockSize_*sizeof(T));
// 			const T *const end = ptrTable[j] + i;
// 			const T *J = std::begin(rhs) + j*blockSize_;
// 			for (T *I = ptrTable[j]; I!=end; ++I, ++J)
// 				new(I) T{*J};
// 		}
// 	}

// 	template<size_t blksize, size_t blknum>
// 	constexpr StableArray &operator =(const StableArray<T, blksize, blknum> &rhs) noexcept{

// 	}

// 	template<size_t blksize, size_t blknum>
// 	constexpr StableArray &operator =(StableArray<T, blksize, blknum> &&rhs) noexcept{
// 		~StableArray();
// 		std::copy_n(rhs.ptrTable, rhs.filledBlockNumber(), ptrTable);
// 		size_ = std::size(rhs);
// 		rhs.size = 0;
// 	}

// 	~StableArray(){
// 		const size_t lastBlockIndex = size_ / blockSize_;
// 		const size_t lastBlockElements = size_ % blockSize_;
// 		for (size_t i = lastBlockElements; i>0; --i)
// 			~T(ptrTable[lastBlockIndex][i]);
// 		if (lastBlockElements) free(ptrTable[lastBlockIndex]);
// 		for (size_t i=size_/blockSize_; i!=0; --i)
// 			delete ptrTable[i];
// 	}

	
// 	constexpr T &operator [](const uint32_t index) noexcept{ return ptrTable[index/blockSize_][index%blockSize_]; }

// 	[[nodiscard]] constexpr const T &operator [](const uint32_t index) const noexcept{ return ptrTable[index/blockSize_][index%blockSize_]; }

// 	constexpr void pushBack(const T &arg) noexcept;
// 	constexpr void pushBack(T &&arg) noexcept;

// 	constexpr void popBack() noexcept;
// 	constexpr void popBack(uint32_t count) noexcept;

// 	constexpr void resize(const size_t newSize) noexcept{
// 		for (size_t i=this->capacity(); i<newSize; i+=blockSize_)
// 			ptrTable[i] = (T *)aligned_alloc(alignof(T), blockSize_*sizeof(T));
// 		size_ = newSize;
// 	}

// 	[[nodiscard]] constexpr size_t size() const noexcept{ return size_; }
// 	[[nodiscard]] constexpr size_t capacity() const noexcept{ return (size_+blockSize_-1)/blockSize_*blockSize_; }
// 	[[nodiscard]] constexpr size_t filledBlockNumber() const noexcept{ return (size_+blockSize_-1)/blockSize_; }

// 	[[nodiscard]] constexpr size_t blockSize() const noexcept{ return blockSize_; }
// 	[[nodiscard]] constexpr size_t maxBlockNumber() const noexcept{ return maxBlockNumber_; }
// 	[[nodiscard]] constexpr size_t totCapacity() const noexcept{ return blockSize_*maxBlockNumber_; }

// 	[[nodiscard]] constexpr bool isFull() const noexcept{ return size_ >= capacity(); }

// 	constexpr T &front() noexcept{ return ptrTable[0][0]; }
// 	constexpr T &back() noexcept{ return ptrTable[(size_-1)/blockSize_][(size_-1)%blockSize_]; }
// 	[[nodiscard]] constexpr const T &front() const noexcept{ return ptrTable[0][0]; }
// 	[[nodiscard]] constexpr const T &back() const noexcept{ ptrTable[(size_-1)/blockSize_][(size_-1)%blockSize_]; }


// 	struct It : public std::iterator<std::random_access_iterator_tag, T>{
// 		[[nodiscard]] constexpr It operator +(const size_t steps) const noexcept{ return It{src, index + steps}; }
// 		[[nodiscard]] constexpr It operator -(const size_t steps) const noexcept{ return It{src, index - steps}; }
// 		constexpr It &operator +=(const size_t steps) noexcept{ index += steps; return *this; }
// 		constexpr It &operator -=(const size_t steps) noexcept{ index -= steps; return *this; }
// 		constexpr It &operator ++() noexcept{ ++index; return *this; }
// 		constexpr It &operator --() noexcept{ --index; return *this; }
// 		constexpr T &operator *() noexcept{ return (*src)[index]; }

// 		[[nodiscard]] constexpr int64_t operator -(const It iter) const noexcept{ return index - iter.index; }

// 		[[nodiscard]] constexpr bool operator ==(const It iter) const{ return src==iter.src && index==iter.index; }
// 		[[nodiscard]] constexpr bool operator !=(const It iter) const{ return src!=iter.src || index!=iter.index; }

// 		StableArray<T, blockSize_, maxBlockNumber_> *src;
// 		size_t index;
// 	};

// 	constexpr T *begin() noexcept{ return It{this, 0}; }
// 	constexpr T *end() noexcept{return It{this, size_}; }

// 	[[nodiscard]] constexpr const T *cbegin() const noexcept{ return It{this, 0}; }
// 	[[nodiscard]] constexpr const T *cend() const noexcept{ return It{this, size_}; }


// 	typedef T value_type;
// private:
// 	size_t size_;
// 	T *ptrTable[maxBlockNumber_];
// };


}	// END OF NAMESPACE	///////////////////////////////////////////////////////////////////