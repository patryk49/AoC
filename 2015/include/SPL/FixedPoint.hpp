#include "SPL/Utils.hpp"

namespace sp{


template<class IntType> struct HigherIntResolution;
template<> struct HigherIntResolution<int8_t>{ typedef int16_t type; };
template<> struct HigherIntResolution<int16_t>{ typedef int32_t type; };
template<> struct HigherIntResolution<int32_t>{ typedef int64_t type; };
template<> struct HigherIntResolution<int64_t>{ typedef int64_t type; };
template<> struct HigherIntResolution<uint8_t>{ typedef uint16_t type; };
template<> struct HigherIntResolution<uint16_t>{ typedef uint32_t type; };
template<> struct HigherIntResolution<uint32_t>{ typedef uint64_t type; };
template<> struct HigherIntResolution<uint64_t>{ typedef uint64_t type; };



template<uint32_t fracBits = 16, class BaseT = int32_t>
class FixedPoint{
	static_assert(std::is_integral<BaseT>() && std::is_signed<BaseT>(), "Fixed point must be built on top of signed iteger.");
	static_assert(fracBits < std::numeric_limits<BaseT>::digits, "Number of fractional bits must be smaller than total number of bits.");
public:
	FixedPoint() noexcept {}
	FixedPoint(const FixedPoint &) = default;
	FixedPoint &operator =(const FixedPoint &) = default;

	template<uint32_t fracBits1>
	constexpr auto &operator +=(const FixedPoint<fracBits1, BaseT> rhs){
		data += ((FixedPoint<fracBits, BaseT>)rhs).data;
		return *this;
	}
	template<uint32_t fracBits1>
	constexpr auto &operator -=(const FixedPoint<fracBits1, BaseT> rhs){
		data -= ((FixedPoint<fracBits, BaseT>)rhs).data;
		return *this;
	}
	template<uint32_t fracBits1>
	constexpr auto &operator *=(const FixedPoint<fracBits1, BaseT> rhs){
		data = ((typename HigherIntResolution<BaseT>::type)data *
			(typename HigherIntResolution<BaseT>::type)((FixedPoint<fracBits, BaseT>)rhs).data) >> fracBits;
		return *this;
	}
	template<uint32_t fracBits1>
	constexpr auto &operator /=(const FixedPoint<fracBits1, BaseT> rhs){
		data = (((typename HigherIntResolution<BaseT>::type)data << fracBits) /
			(typename HigherIntResolution<BaseT>::type)((FixedPoint<fracBits, BaseT>)rhs).data);
		return *this;
	}
	template<uint32_t fracBits1>
	constexpr auto &operator %=(const FixedPoint<fracBits1, BaseT> rhs){
		data %= ((FixedPoint<fracBits, BaseT>)rhs).data;
		return *this;
	}

	template<uint32_t fracBits1>
	constexpr auto &operator |=(const FixedPoint<fracBits1, BaseT> rhs){
		data |= ((FixedPoint<fracBits, BaseT>)rhs).data;
		return *this;
	}
	template<uint32_t fracBits1>
	constexpr auto &operator &=(const FixedPoint<fracBits1, BaseT> rhs){
		data &= ((FixedPoint<fracBits, BaseT>)rhs).data;
		return *this;
	}
	template<uint32_t fracBits1>
	constexpr auto &operator ^=(const FixedPoint<fracBits1, BaseT> rhs){
		data ^= ((FixedPoint<fracBits, BaseT>)rhs).data;
		return *this;
	}

// CONVERSIONS
	template<uint32_t fracBits1, class BaseT1>
	constexpr FixedPoint(const FixedPoint<fracBits1, BaseT1> x) noexcept{
		constexpr int pointsDistance = fracBits1 - fracBits;
		if constexpr (pointsDistance >= 0)
			data = (BaseT)x.data >> pointsDistance;
		else
			data = (BaseT)x.data << -pointsDistance;
	}

	template<class T, class = std::enable_if_t<std::is_arithmetic_v<T>, T>>
	constexpr FixedPoint(const T x) noexcept : data{std::is_integral_v<T> ? (BaseT)x << fracBits : (BaseT)(x * (T)(1 << fracBits))} {}


	constexpr operator BaseT() const noexcept{ return (BaseT)(data >> fracBits); }
	constexpr operator float() const noexcept{ return (float)data / (float)(1 << fracBits); }
	constexpr operator double() const noexcept{ return (double)data / (double)(1 << fracBits); }
	constexpr operator long double() const noexcept{ return (long double)data / (long double)(1 << fracBits); }

	typedef BaseT Base;
	static constexpr uint32_t fractionalBits = fracBits;

	BaseT data;
};







template<uint32_t fracBits1, uint32_t fracBits2, class BaseT>
constexpr auto operator +(const FixedPoint<fracBits1, BaseT> lhs, const FixedPoint<fracBits2, BaseT> rhs){
	typedef FixedPoint<std::max(fracBits1, fracBits2), BaseT> ResultType;
	ResultType result;
	result.data = ((ResultType)lhs).data + ((ResultType)rhs).data;
	return result;
}
template<uint32_t fracBits1, uint32_t fracBits2, class BaseT>
constexpr auto operator -(const FixedPoint<fracBits1, BaseT> lhs, const FixedPoint<fracBits2, BaseT> rhs){
	typedef FixedPoint<std::max(fracBits1, fracBits2), BaseT> ResultType;
	ResultType result;
	result.data = ((ResultType)lhs).data - ((ResultType)rhs).data;
	return result;
}
template<uint32_t fracBits1, uint32_t fracBits2, class BaseT>
constexpr auto operator *(const FixedPoint<fracBits1, BaseT> lhs, const FixedPoint<fracBits2, BaseT> rhs){
	typedef FixedPoint<std::max(fracBits1, fracBits2), BaseT> ResultType;
	ResultType result;
	result.data = ((typename HigherIntResolution<BaseT>::type)((ResultType)lhs).data *
		(typename HigherIntResolution<BaseT>::type)((ResultType)rhs).data) >> ResultType::fractionalBits;
	return result;
}
template<uint32_t fracBits1, uint32_t fracBits2, class BaseT>
constexpr auto operator /(const FixedPoint<fracBits1, BaseT> lhs, const FixedPoint<fracBits2, BaseT> rhs){
	typedef FixedPoint<std::max(fracBits1, fracBits2), BaseT> ResultType;
	ResultType result;
	result.data = (((typename HigherIntResolution<BaseT>::type)((ResultType)lhs).data << ResultType::fractionalBits) /
		(typename HigherIntResolution<BaseT>::type)((ResultType)rhs).data);
	return result;
}
template<uint32_t fracBits1, uint32_t fracBits2, class BaseT>
constexpr auto operator %(const FixedPoint<fracBits1, BaseT> lhs, const FixedPoint<fracBits2, BaseT> rhs){
	typedef FixedPoint<std::max(fracBits1, fracBits2), BaseT> ResultType;
	ResultType result;
	result.data = ((ResultType)lhs).data % ((ResultType)rhs).data;
	return result;
}

template<uint32_t fracBits1, uint32_t fracBits2, class BaseT>
constexpr auto operator |(const FixedPoint<fracBits1, BaseT> lhs, const FixedPoint<fracBits2, BaseT> rhs){
	typedef FixedPoint<std::max(fracBits1, fracBits2), BaseT> ResultType;
	ResultType result;
	result.data = ((ResultType)lhs).data | ((ResultType)rhs).data;
	return result;
}
template<uint32_t fracBits1, uint32_t fracBits2, class BaseT>
constexpr auto operator &(const FixedPoint<fracBits1, BaseT> lhs, const FixedPoint<fracBits2, BaseT> rhs){
	typedef FixedPoint<std::max(fracBits1, fracBits2), BaseT> ResultType;
	ResultType result;
	result.data = ((ResultType)lhs).data & ((ResultType)rhs).data;
	return result;
}
template<uint32_t fracBits1, uint32_t fracBits2, class BaseT>
constexpr auto operator ^(const FixedPoint<fracBits1, BaseT> lhs, const FixedPoint<fracBits2, BaseT> rhs){
	typedef FixedPoint<std::max(fracBits1, fracBits2), BaseT> ResultType;
	ResultType result;
	result.data = ((ResultType)lhs).data ^ ((ResultType)rhs).data;
	return result;
}
template<uint32_t fracBits, class BaseT>
constexpr auto operator ~(const FixedPoint<fracBits, BaseT> x){
	typedef FixedPoint<fracBits, BaseT> ResultType;
	ResultType result;
	result.data = ~x.data;
	return result;
}

template<uint32_t fracBits1, uint32_t fracBits2, class BaseT>
constexpr bool operator ==(const FixedPoint<fracBits1, BaseT> lhs, const FixedPoint<fracBits2, BaseT> rhs){
	typedef FixedPoint<std::max(fracBits1, fracBits2), BaseT> ResultType;
	return ((ResultType)lhs).data == ((ResultType)rhs).data;
}
template<uint32_t fracBits1, uint32_t fracBits2, class BaseT>
constexpr bool operator !=(const FixedPoint<fracBits1, BaseT> lhs, const FixedPoint<fracBits2, BaseT> rhs){
	typedef FixedPoint<std::max(fracBits1, fracBits2), BaseT> ResultType;
	return ((ResultType)lhs).data != ((ResultType)rhs).data;
}
template<uint32_t fracBits1, uint32_t fracBits2, class BaseT>
constexpr bool operator >(const FixedPoint<fracBits1, BaseT> lhs, const FixedPoint<fracBits2, BaseT> rhs){
	typedef FixedPoint<std::max(fracBits1, fracBits2), BaseT> ResultType;
	return ((ResultType)lhs).data >= ((ResultType)rhs).data;
}
template<uint32_t fracBits1, uint32_t fracBits2, class BaseT>
constexpr bool operator <(const FixedPoint<fracBits1, BaseT> lhs, const FixedPoint<fracBits2, BaseT> rhs){
	typedef FixedPoint<std::max(fracBits1, fracBits2), BaseT> ResultType;
	return ((ResultType)lhs).data <= ((ResultType)rhs).data;
}
template<uint32_t fracBits1, uint32_t fracBits2, class BaseT>
constexpr bool operator >=(const FixedPoint<fracBits1, BaseT> lhs, const FixedPoint<fracBits2, BaseT> rhs){
	typedef FixedPoint<std::max(fracBits1, fracBits2), BaseT> ResultType;
	return ((ResultType)lhs).data >= ((ResultType)rhs).data;
}
template<uint32_t fracBits1, uint32_t fracBits2, class BaseT>
constexpr bool operator <=(const FixedPoint<fracBits1, BaseT> lhs, const FixedPoint<fracBits2, BaseT> rhs){
	typedef FixedPoint<std::max(fracBits1, fracBits2), BaseT> ResultType;
	return ((ResultType)lhs).data <= ((ResultType)rhs).data;
}
template<uint32_t fracBits, class BaseT>
constexpr bool operator !(const FixedPoint<fracBits, BaseT> x){
	return !x.data;
}



} // END OF NAMESPACE //////////////////////////////////////////////////////////////////////////////////////////