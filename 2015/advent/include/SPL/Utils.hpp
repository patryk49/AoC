#pragma once

#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <algorithm>
#include <vector>
#include <array>
#include <functional>
#include <numeric>


#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstrict-aliasing"

namespace sp{ // BEGINING OF NAMESPACE //////////////////////////////////////////////////////////////////



// TYPE TRAITS
namespace priv__{
	template<uint32_t size> struct IntOfGivenSizeStruct;
	template<> struct IntOfGivenSizeStruct<1>{ typedef int8_t type; };
	template<> struct IntOfGivenSizeStruct<2>{ typedef int16_t type; };
	template<> struct IntOfGivenSizeStruct<4>{ typedef int32_t type; };
	template<> struct IntOfGivenSizeStruct<8>{ typedef int64_t type; };

	template<uint32_t size> struct UIntOfGivenSizeStruct;
	template<> struct UIntOfGivenSizeStruct<1>{ typedef uint8_t type; };
	template<> struct UIntOfGivenSizeStruct<2>{ typedef uint16_t type; };
	template<> struct UIntOfGivenSizeStruct<4>{ typedef uint32_t type; };
	template<> struct UIntOfGivenSizeStruct<8>{ typedef uint64_t type; };
}

template<uint32_t size> using IntOfGivenSize = priv__::UIntOfGivenSizeStruct<size>;
template<uint32_t size> using UIntOfGivenSize = priv__::UIntOfGivenSizeStruct<size>;

template<class T>
constexpr bool isTriviallyPassabe = sizeof(T)<=16 && std::is_trivially_constructible_v<T>;




// INDEXED STRUCT
template<class T, class... TRest>
struct IndexedStruct{
	static constexpr size_t argsCount = sizeof...(TRest) + 1;
	typedef T value_type;
	IndexedStruct<TRest ...> next;
	T data;
};

template<class T>
struct IndexedStruct<T>{
	static constexpr size_t argsCount = 1;
	typedef T value_type;
	T data;
};






// GETTING SINGLE ARGUMENT FROM ARGUMENT PACK
template<uint32_t index, class Arg0_t, class... Args_t>
constexpr const auto &getArg(const Arg0_t &arg0, const Args_t &...args) noexcept{
	if constexpr (index)
		return getArg<index-1>(args...);
	else
		return arg0;
}
template<uint32_t index, class Arg0_t>
constexpr const auto &getArg(const Arg0_t &arg0) noexcept{ return arg0; }

template<uint32_t index, class Arg0_t, class... Args_t>
constexpr auto &getArg(Arg0_t &arg0, Args_t &...args) noexcept{
	if constexpr (index)
		return getArg<index-1>(args...);
	else
		return arg0;
}
template<uint32_t index, class Arg0_t>
constexpr auto &getArg(Arg0_t &arg0) noexcept{ return arg0; }







// CONVERTING UNIONS
union ConvF{
	uint32_t i;
	float f;
};
union ConvUninonD{
	uint64_t i;
	double d;
};
template<class T>	// CONVERT TYPE TO AN UNSIGNED INTEGER
union ConvUnion{
	T t;
	UIntOfGivenSize<sizeof(T)> u;
};







// INTEGER MATH
template<class T>
[[nodiscard]] constexpr T sign(const T x) noexcept{
	return (T)((1 - (signbit(x)<<1)) & -(x != T{}));
}

template<class T>
[[nodiscard]] constexpr int signInt(const T x) noexcept{
	return (1 - (signbit(x)<<1)) & -(x != T{});
}

template<class T>
[[nodiscard]] constexpr T gcd(T m, T n) noexcept{
	if constexpr (std::is_signed_v<T>){
		m = abs(m);
		n = abs(n);
	}
	while (m && n){
		const uint32_t temp = m;
		m = n;
		n = temp % n;
	}
   return n | m;
}

template<class T>
[[nodiscard]] constexpr T fastGcd(T m, T n) noexcept{
	if constexpr (std::is_signed_v<T>){
		m = abs(m);
		n = abs(n);
	}
	if (!(m && n)) return T{};
	while (m != n){
		const T mask = -(n < m);
		m -= n & mask;
		n -= m & ~mask;
	}
   return m;
}

[[nodiscard]] constexpr int factorial(uint32_t n) noexcept{
	uint32_t result = n;
	while (--n) result *= n;
	return result;
}

template<class T>
[[nodiscard]] constexpr std::enable_if_t<std::is_integral_v<T>, T> intSqrt(T x) noexcept{
	T one = 1 << 30;
	while (one > x) one >>= 2;
	T res = 0;
	while (one){
		const T cond = -(x >= res + one);
		x -= (res + one) & cond;
		res += (one << 1) & cond;

		res >>= 1;
		one >>= 2;
	}
	return res;
}

[[nodiscard]] constexpr uint32_t roundUpTo2Power(uint32_t x) noexcept{
	--x;
	x |= x >> 1;
	x |= x >> 2;
	x |= x >> 4;
	x |= x >> 8;
	x |= x >> 16;
	return x + 1;
}

[[nodiscard]] constexpr uint32_t roundDownTo2Power(uint32_t x) noexcept{
	x |= x >> 1;
	x |= x >> 2;
	x |= x >> 4;
	x |= x >> 8;
	x |= x >> 16;
	return x - (x >> 1);
}

[[nodiscard]] constexpr uint32_t countOneBits(uint32_t x) noexcept{
	x -= (x >> 1) & 0x55555555;
	x = (x & 0x33333333) + ((x >> 2) & 0x33333333);
	x = (x + (x >> 4)) & 0x0F0F0F0F;
	x += x >> 8;
	x += x >> 16;
	return x & 0x0000003F;
}










// FLOATING POINT MATH
[[nodiscard]] constexpr int32_t getMantissa(const float x) noexcept{
	if (signbit(x))
		return -((1<<23) | (*(int32_t *)&x & 0x007fffff));
	return (1<<23) | (*(int32_t *)&x & 0x007fffff);
}

[[nodiscard]] constexpr int32_t getExponent(const float x) noexcept{
	return ((*(int32_t *)&x & 0x7fffffff) >> 23) - 0x7f;
}

[[nodiscard]] constexpr int64_t getMantissa(const double x) noexcept{
	if (signbit(x))
		return -((1ll<<52) | (*(int64_t *)&x & 0x000fffffffffffff));
	return (1ll<<52) | (*(int64_t *)&x & 0x000fffffffffffff);
}

[[nodiscard]] constexpr int64_t getExponent(const double x) noexcept{
	return ((*(int64_t *)&x & 0x7fffffffffffffff) >> 52) - 0x3ff;
}








// CONDITIONAL ARITMETIC
template<class T>
[[nodiscard]] constexpr std::enable_if_t<std::is_arithmetic_v<T>, T>
enable(const bool condition, const T value) noexcept{
	return -(IntOfGivenSize<roundUpTo2Power(sizeof(T))>)condition & value;
}

template<class T>
[[nodiscard]] constexpr std::enable_if_t<std::is_arithmetic_v<T>, T>
choose(const bool condition, const T valueOnTrue, const T valueOnFalse) noexcept{
	return (-(IntOfGivenSize<roundUpTo2Power(sizeof(T))>)condition & valueOnTrue) |
		(((IntOfGivenSize<roundUpTo2Power(sizeof(T))>)condition-1) & valueOnFalse);
}







// TYPE ERASURES
template<class Signature> struct FunctionRef;

template<class Res, class... Args>
struct FunctionRef<Res(Args...)>{
	constexpr FunctionRef() noexcept : functionPtr{nullptr}, objectPtr{nullptr} {}
	FunctionRef(const FunctionRef &) noexcept = default;
	FunctionRef(FunctionRef &&) noexcept = default;
	FunctionRef &operator =(const FunctionRef &) noexcept = default;
	FunctionRef &operator =(FunctionRef &&) noexcept = default;
	~FunctionRef() noexcept = default;

	template<class Func>
	constexpr FunctionRef(Func *const func) noexcept{
		static_assert(std::is_invocable<Func, Args...>(), "Function's arguments don't match with thoose declared in template parameters.");
		if constexpr (std::is_function_v<Func>){
			functionPtr = (void *)func;
			objectPtr = nullptr;
		} else{
	#pragma GCC diagnostic push
	#pragma GCC diagnostic ignored "-Wpmf-conversions"
			functionPtr = (void *)(Res (Func::*)(Args...))&Func::operator();
	#pragma GCC diagnostic pop
			objectPtr = func;
		}
	}

	template<class Func>
	constexpr FunctionRef &operator =(Func *const func) noexcept{
		if constexpr (std::is_function_v<Func>){
			objectPtr = nullptr;
			functionPtr = (void *)func;
		}
		if constexpr (std::is_class_v<Func>){
	#pragma GCC diagnostic push
	#pragma GCC diagnostic ignored "-Wpmf-conversions"
			functionPtr = (void *)(Res (Func::*)(Args...))&Func::operator();
	#pragma GCC diagnostic pop
			objectPtr = func;
		}
	}

	constexpr Res operator ()(Args... args) const noexcept{
		if (objectPtr) 
			return ((Res (*)(const void *, Args...))functionPtr)(objectPtr, args...);
		else
			return ((Res (*)(Args...))functionPtr)(args...);
	}
	constexpr Res operator ()(Args... args) noexcept{
		if (objectPtr) 
			return ((Res (*)(void *, Args...))functionPtr)(objectPtr, args...);
		else
			return ((Res (*)(Args...))functionPtr)(args...);
	}

	typedef Res result_type;
	typedef Res arg0_type;

	void *functionPtr;
	void *objectPtr;
};









// FAST MATH
struct Rand32{
	[[nodiscard]] constexpr uint32_t min() const noexcept{ return 0; }

	constexpr uint32_t max() const noexcept{ return UINT32_MAX; }

	[[nodiscard]] constexpr uint32_t operator ()() noexcept{
		seed += 0xe120fc15;
		uint64_t temp = (uint64_t)seed * 0x4a39b70d;
		temp = (uint64_t)((temp >> 32) ^ temp) * 0x12fad5c9;
		return (temp >> 32) ^ temp;
	}

	typedef uint32_t result_type;
	uint32_t seed;
};


[[nodiscard]] constexpr float qLog(const float x) noexcept{
	constexpr float scaleDown = 1.f/(float)0x00800000;
	constexpr ConvF one{.f = 1.f};
	return (float)(*(uint32_t *)&x - one.i)*scaleDown;
}

[[nodiscard]] constexpr float qExp(const float x) noexcept{
	constexpr float scaleUp = 0x00800000;
	constexpr ConvF one{.f = 1.f};
	const ConvF res{.i = (uint32_t)(x*scaleUp) + one.i};
	return res.f;
}

[[nodiscard]] constexpr float qPow(const float x, const float y) noexcept{
	constexpr ConvF one{.f = 1.f};
	ConvF res{.i = (uint32_t)((float)(*(uint32_t *)&x - one.i)*y) + one.i};
	return res.f;
}

[[nodiscard]] constexpr float qSqrt(const float x) noexcept{
	ConvF res{.f = x};
	res.i -= 1<<23;
	res.i >>= 1;
	res.i += 1<<29;
	return (res.f + x/res.f)*0.5f;
}

[[nodiscard]] constexpr float qInvSqrt(const float x) noexcept{
	ConvF res{.i = (uint32_t)0x5f3759df - (*(uint32_t *)&x>>1)};
	return res.f * (1.5f - 0.5f*x*res.f*res.f);
}





// HEAP
template<class It>
void repairHeap(It begin, const It end, const size_t startingIndex) noexcept{
	It child;
	for(size_t i=startingIndex; (child=begin+((i<<1)|1)) < end; i=child-begin){
		child += child[0] < child[child+1 != end];
		
		if (child[0] < begin[i]) return;

		std::iter_swap(begin+i, child);
	}
}

template<class It, class Compare = std::less<typename It::value_type>>
void repairHeap(It begin, const It end, Compare comapre, const size_t startingIndex) noexcept{
	It child;
	for(size_t i=startingIndex; (child=begin+((i<<1)|1)) < end; i=child-begin){
		child += child[0] < child[child+1 != end];
		
		if (child[0] < begin[i]) return;

		std::iter_swap(begin+i, child);
	}
}

template<class It>
void makeHeap(It begin, const It end) noexcept{
	for (int i=(end-begin-1)>>1; i>=0; --i)
		repairHeap(begin, end, i);
}

template<class It, class Compare>
void makeHeap(It begin, const It end, Compare compare) noexcept{
	for (int i=(end-begin-1)>>1; i>=0; --i)
		repairHeap(begin, end, compare, i);
}










// PARTITION
template<class T>
constexpr T *partition(T *const first, T *last, T *const pivot) noexcept{
	if (first >= last) return first;
	std::iter_swap(first, pivot);
	
	T lastVal = std::move(*--last);
	*last = *first;
	
	T *It = first;
	for (;;){
		do ++It; while (*It < *first);
		*last = std::move(*It);
		do --last; while (*first < *last);
		if (It >= last) break;
		*It = std::move(*last);
	}
	if (It == last+2){
		*It = std::move(*(last+1));
		--It;
	}
	{
		T *const partitionPoint = It - (*first < lastVal);
		*It = std::move(lastVal);
		std::iter_swap(first, partitionPoint);
		return partitionPoint;
	}
}





// RADIX SORT FOR UNSIGNED TYPES
template<class T, size_t baseBits = 8>
void radixLSD(T *const first, T *const last) noexcept{
	static_assert(baseBits);
	constexpr size_t base = 1 << baseBits;
	constexpr size_t bitMask = base - 1;
	
	uint32_t counts[base];
	std::vector<T> buffer(last-first);

	using IT = sp::UIntOfGivenSize<sp::roundUpTo2Power(sizeof(T))>;

	for (IT step=0; step!=sizeof(T)*8/baseBits; ++step){
		memset(counts, 0, sizeof(counts));
		if (step & 1){
			for (T *It=&*std::begin(buffer); It!=&*std::end(buffer); ++It)
				++counts[(*(IT *)It >> step*baseBits) & bitMask];

			for (IT *C=std::begin(counts)+1; C!=std::end(counts); ++C) *C += *(C-1);
			
			for (T *It=&*std::end(buffer)-1; It!=&*std::begin(buffer)-1; --It)
				first[--counts[(*(IT *)It >> step*baseBits) & bitMask]] = *It;

		} else{
			for (T *It=first; It!=last; ++It)
				++counts[(*(IT *)It >> step*baseBits) & bitMask];

			for (IT *C=std::begin(counts)+1; C!=std::end(counts); ++C) *C += *(C-1);
			
			for (T *It=last-1; It!=first-1; --It)
				buffer[--counts[(*(IT *)It >> step*baseBits) & bitMask]] = *It;
		}
	}
}







// RANGES
namespace priv__{

template<class T>
struct RangeClass{
	struct It : public std::iterator<std::forward_iterator_tag, T>{
		constexpr T operator *() const noexcept{ return i; }
		constexpr const T &operator ++() noexcept{ i += step; return i; }
		constexpr bool operator !=(const T rhs) const noexcept{ return i <= rhs; }

		T i;
		T step;
	};

	constexpr It begin() const noexcept{ return It{{}, firstElement, stepSize}; }
	constexpr T end() const noexcept{ return lastElement; }

	T firstElement;
	T lastElement;
	T stepSize;
};

template<class T>
struct RangeRClass{
	struct It : public std::iterator<std::forward_iterator_tag, T>{
		constexpr T operator *() const noexcept{ return i; }
		constexpr const T &operator ++() noexcept{ i -= step; return i; }
		constexpr bool operator !=(const T rhs) const noexcept{ return i >= rhs; }

		T i;
		T step;
	};

	constexpr It begin() const noexcept{ return It{{}, firstElement, stepSize}; }
	constexpr T end() const noexcept{ return lastElement; }

	T firstElement;
	T lastElement;
	T stepSize;
};

} // END OF NAMESPACE PRIV

template<class T>
std::enable_if_t<std::is_integral_v<T> || std::is_floating_point_v<T>, priv__::RangeClass<T>>
Range(const T first, const T last, const T step = T{1}) noexcept{
	return priv__::RangeClass<T>{first, last, step};
};

template<class T>
std::enable_if_t<std::is_integral_v<T> || std::is_floating_point_v<T>, priv__::RangeClass<T>> 
RangeR(const T first, const T last, const T step = T{1}) noexcept{
	return priv__::RangeRClass<T>{first, last, step};
};

}	// END OF NAMESPACE	///////////////////////////////////////////////////////////////////