#pragma once

#include "Utils.hpp"
#include <initializer_list>
#include <type_traits>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wclass-memaccess"

namespace sp{ // BEGINING OF NAMESPACE //////////////////////////////////////////////////////////////////


template<class T>
struct SOAView{
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

	constexpr void expandBy(const size_t count) noexcept{ endPtr += count; }
	constexpr void skrinkBy(size_t count) noexcept{ endPtr -= count; }


	typedef T value_type;

	T *beginPtr, *endPtr;
};













template<class T0, class T1, size_t cap>
struct SOAFiniteArray{
	static_assert(cap, "this makes no sense");

	constexpr SOAFiniteArray() noexcept : arrSize{} {}
	constexpr SOAFiniteArray(const size_t size) noexcept : arrSize{size} {
		if constexpr (!std::is_trivially_default_constructible_v<T0>)
			for (T0 *I=mem0; I!=mem0+arrSize; ++I) new(I) T0();
		if constexpr (!std::is_trivially_default_constructible_v<T1>)
			for (T1 *I=mem1; I!=mem1+arrSize; ++I) new(I) T1();
	}
	~SOAFiniteArray() noexcept{
		for (T0 *I=mem0; I!=mem0+arrSize; ++I) I->~T0();
		for (T1 *I=mem1; I!=mem1+arrSize; ++I) I->~T1();
	}

	template<size_t cap1>
	constexpr SOAFiniteArray(const SOAFiniteArray<T0, T1, cap1> &rhs) noexcept : arrSize{rhs.arrSize} {
		const T0 *K = rhs.mem0;
		const T1 *L = rhs.mem1;
		for (T0 *I=mem0, *J=mem1; I!=mem0+arrSize; ++I, ++J, ++K, ++L){
			new(I) T0(*K);
			new(J) T0(*L);
		}
	}
	template<size_t cap1>
	constexpr SOAFiniteArray(SOAFiniteArray<T0, T1, cap1> &&rhs) noexcept : arrSize{rhs.arrSize} {
		const T0 *K = rhs.mem0;
		const T1 *L = rhs.mem1;
		T0 *I = mem0;
		T1 *J = mem1;
		for (; I!=mem0+arrSize; ++I, ++J, ++K, ++L){
			new(I) T0(std::move(*K));
			new(J) T0(std::move(*L));
		}
		rhs.arrSize = 0;
	}

	template<size_t cap1>
	constexpr SOAFiniteArray &operator =(const SOAFiniteArray<T0, T1, cap1> &rhs) noexcept{
		T0 *I = mem0;
		T1 *J = mem1;
		const T0 *K = rhs.mem0;
		const T1 *L = rhs.mem1;
		T0 *const endPtr = mem0 + std::min(arrSize, rhs.arrSize); 
		for (; I!=endPtr; ++I, ++J, ++K, ++L){
			*I = *K;
			*J = *L;
		}
		if (rhs.arrSize > arrSize)
			for (; I!=mem0+rhs.arrSize; ++I, ++J, ++K, ++L){
				new(I) T0(*K);
				new(J) T1(*L);
			}
		else 
			for (; I!=mem0+arrSize; ++I, ++J){
				I->~T0();
				J->~T1();
			}
		arrSize = rhs.arrSize;
		return *this;
	}
	template<size_t cap1>
	constexpr SOAFiniteArray &operator =(SOAFiniteArray<T0, T1, cap1> &&rhs) noexcept{
		T0 *I = mem0;
		T1 *J = mem1;
		const T0 *K = rhs.mem0;
		const T1 *L = rhs.mem1;
		T0 *const endPtr = mem0 + std::min(arrSize, rhs.arrSize); 
		for (; I!=endPtr; ++I, ++J, ++K, ++L){
			*I = std::move(*K);
			*J = std::move(*L);
		}
		if (rhs.arrSize > arrSize)
			for (; I!=mem0+rhs.arrSize; ++I, ++J, ++K, ++L){
				new(I) T0(std::move(*K));
				new(J) T1(std::move(*L));
			}
		else 
			for (; I!=mem0+arrSize; ++I, ++J){
				I->~T0();
				J->~T1();
			}
		arrSize = rhs.arrSize;
		rhs.arrSize = 0;
		return *this;
	}


	[[nodiscard]] constexpr bool isFull() const noexcept{ return arrSize >= cap; }

	[[nodiscard]] constexpr size_t size() const noexcept{ return arrSize; }
	[[nodiscard]] constexpr size_t capacity() const noexcept{ return cap; }

	constexpr T0 *begin0() noexcept{ return mem0; }
	constexpr T0 *end0() noexcept{ return mem0 + arrSize; }
	[[nodiscard]] constexpr const T0 *begin0() const noexcept{ return mem0; }
	[[nodiscard]] constexpr const T0 *end0() const noexcept{ return mem0 + arrSize; }
	[[nodiscard]] constexpr const T0 *cbegin0() const noexcept{ return mem0; }
	[[nodiscard]] constexpr const T0 *cend0() const noexcept{ return mem0 + arrSize; }

	constexpr T0 *begin1() noexcept{ return mem1; }
	constexpr T0 *end1() noexcept{ return mem1 + arrSize; }
	[[nodiscard]] constexpr const T1 *begin1() const noexcept{ return mem1; }
	[[nodiscard]] constexpr const T1 *end1() const noexcept{ return mem1 + arrSize; }
	[[nodiscard]] constexpr const T1 *cbegin1() const noexcept{ return mem1; }
	[[nodiscard]] constexpr const T1 *cend1() const noexcept{ return mem1 + arrSize; }


	constexpr void push_back(const T0 &arg0, const T1 &arg1) noexcept{
		new(mem0+arrSize) T0(arg0);
		new(mem1+arrSize) T1(arg1);
		++arrSize;
	}
	constexpr void push_back(T0 &&arg0, T1 &&arg1) noexcept{
		new(mem0+arrSize) T0(std::move(arg0));
		new(mem1+arrSize) T1(std::move(arg1));
		++arrSize;
	}

	constexpr void pop_back() noexcept{
		--arrSize;
		(mem0 + arrSize)->~T0();
		(mem1 + arrSize)->~T1();
	}

	constexpr void expandBy(const size_t count) noexcept{
		size += count;
		if constexpr (!std::is_trivially_default_constructible_v<T0>)
			for (T0 *I=mem0+arrSize-count; I!=mem0+arrSize; ++I) new(I) T0();
		if constexpr (!std::is_trivially_default_constructible_v<T1>)
			for (T1 *I=mem1+arrSize-count; I!=mem1+arrSize; ++I) new(I) T1();
	}
	constexpr void skrinkBy(size_t count) noexcept{
		T0 *I = mem0 + arrSize - count;
		T1 *J = mem1 + arrSize - count;
		for (; I!=mem0+arrSize; ++I, ++J){
			I->~T0();
			J->~T1();
		}
		arrSize -= count;
	}


	constexpr void resize(const size_t size) noexcept{
		if (size > arrSize){
			if constexpr (!std::is_trivially_default_constructible_v<T0>)
				for (T0 *I=mem0+arrSize; I!=mem0+size; ++I) new(I) T0();
			if constexpr (!std::is_trivially_default_constructible_v<T1>)
				for (T1 *I=mem1+arrSize; I!=mem1+size; ++I) new(I) T1();
		} else{
			T0 *I = mem0 + size;
			T1 *J = mem1 + size;
			for (; I!=mem0+arrSize; ++I, ++J){
				I->~T0();
				J->~T1();
			}
		}

		arrSize = size;
	}


//	typedef T value_type;

	union{
		T0 mem0[cap];
	};
	union{
		T1 mem1[cap];
	};
private:
	size_t arrSize;
};















template<class T0, class T1>
struct SOADArray{
	constexpr SOADArray() noexcept : mem0{}, mem1{}, arrSize{}, cap{} {}
	constexpr SOADArray(const size_t size) noexcept :
		beginPtr{(T *)aligned_alloc(alignof(T), size*sizeof(T))}, endPtr{beginPtr+size}, memEndPtr{endPtr}
	{	
		if constexpr (!std::is_trivially_default_constructible_v<T>)
			for (T *I=beginPtr; I!=endPtr; ++I) new(I) T();
	}
	~SOADArray() noexcept{
		for (T *I=beginPtr; I!=endPtr; ++I) I->~T();
		if (beginPtr) free(beginPtr);
	}

	constexpr SOADArray(const SOADArray<T> &rhs) noexcept :
		beginPtr{(T *)aligned_alloc(alignof(T), std::size(rhs)*sizeof(T))}, endPtr{beginPtr+std::size(rhs)}, memEndPtr{endPtr}
	{
		const T *J = rhs.beginPtr;
		for (T *I=beginPtr; I!=endPtr; ++I, ++J) new(I) T(*J);
	}
	constexpr SOADArray(SOADArray<T> &&rhs) noexcept : beginPtr{rhs.beginPtr}, endPtr{rhs.endPtr}, memEndPtr{rhs.memEndPtr} {
		rhs.beginPtr = rhs.endPtr = rhs.memEndPtr = 0;
	}

	constexpr SOADArray(const std::initializer_list<T> &rhs) noexcept :
		beginPtr{(T *)aligned_alloc(alignof(T), std::size(rhs)*sizeof(T))},
		endPtr{beginPtr+std::size(rhs)}, memEndPtr{endPtr}
	{
		const T* I = &*std::begin(rhs); 
		for (T *J=beginPtr; J!=endPtr; ++I, ++J) new(J) T(*I);
	}

	template<class It>
	constexpr SOADArray(It first, const It last) noexcept :
		beginPtr{(T *)aligned_alloc(alignof(T), (char *)last-(char *)first)},
		endPtr{beginPtr+((char *)last-(char *)first)}, memEndPtr{endPtr}
	{
		for (T *I=beginPtr; I!=endPtr; ++I, ++first) new(I) T(*first);
	}

	constexpr SOADArray &operator =(const SOADArray<T> &rhs) noexcept{
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
	constexpr SOADArray &operator =(SOADArray<T> &&rhs) noexcept{
		this->~SOADArray();
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


	T *mem0;
	T *mem1;
private:
	size_t arrSize;
	size_t cap;
};















template<class T, size_t buffSize = (sizeof(T)+15) / sizeof(T)>
struct SOASBArray{
	static_assert(buffSize, "this makes no sense");

// if the most significant bit of dataSize is 1, the the data is separately allocated
	static constexpr size_t allocFlagMask = (size_t)1 << (sizeof(size_t)*8-1);
	static constexpr size_t sizeMask = ~allocFlagMask;

	constexpr SOASBArray() noexcept : dataSize{} {}

	constexpr SOASBArray(const size_t size) noexcept{
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
	constexpr SOASBArray(const SOASBArray<T, buffSize1> &rhs) noexcept{
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
	constexpr SOASBArray(SOASBArray<T, buffSize1> &&rhs) noexcept {
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

	constexpr SOASBArray(const std::initializer_list<T> &rhs) noexcept{
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
	constexpr SOASBArray(It first, const It last) noexcept{
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
	constexpr SOASBArray &operator =(const SOASBArray<T, buffSize1> &rhs) noexcept{
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
	constexpr SOASBArray &operator =(SOASBArray<T, buffSize1> &&rhs) noexcept{
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
			this->~SOASBArray();
			dataSize = rhs.dataSize;
			dataPtr = rhs.dataPtr;
			dynCap = rhs.dynCap;
		}
		rhs.dataSize = 0;
		return *this;
	}

	~SOASBArray() noexcept{
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
struct SOAPtrArray{
	SOAPtrArray() noexcept : ptr{nullptr} {}
	SOAPtrArray(const SOAPtrArray &x) noexcept{
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
	SOAPtrArray(SOAPtrArray &&x) noexcept{
		ptr = x.ptr;
		x.ptr = nullptr; 
	}
	SOAPtrArray &operator =(const SOAPtrArray &x) noexcept{
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
	SOAPtrArray &operator =(SOAPtrArray &&x) noexcept{
		this->~SOAPtrArray();
		ptr = x.ptr;
		x.ptr = nullptr; 
	}
	~SOAPtrArray(){
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