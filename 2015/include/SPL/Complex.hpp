#pragma once

#include "Utils.hpp"

namespace sp{

template<class T = double>
struct Complex{
	T real, imag;
	typedef T valType;
	typedef T value_type;

	constexpr Complex<T> &operator +=(const Complex<T> rhs) noexcept{
		this->real += rhs.real;
		this->imag += rhs.imag;
		return *this;
	}
	constexpr Complex<T> &operator -=(const Complex<T> rhs) noexcept{
		this->real -= rhs.real;
		this->imag -= rhs.imag;
		return *this;
	}
	constexpr Complex<T> &operator *=(const Complex<T> rhs) noexcept{
		const T realHolder = this->real;
		this->real = realHolder*rhs.real - this->imag*rhs.imag;
		this->imag = realHolder*rhs.imag + this->imag*rhs.real;
		return *this;
	}	
	constexpr Complex<T> &operator /=(const Complex<T> rhs) noexcept{
		const T divider = rhs.real*rhs.real + rhs.imag*rhs.imag;
		this->real = (this->real*rhs.real + this->imag*rhs.imag) / divider;
		this->imag = (this->imag*rhs.real - this->real*rhs.imag) / divider;
		return *this;
	}

	constexpr Complex<T> &operator +=(const T rhs) noexcept{
		this->real += rhs;
		return *this;
	}
	constexpr Complex<T> &operator -=(const T rhs) noexcept{
		this->real -= rhs;
		return *this;
	}
	constexpr Complex<T> &operator *=(const T rhs) noexcept{
		this->real *= rhs;
		this->imag *= rhs;
		return *this;
	}	
	constexpr Complex<T> &operator /=(const T rhs) noexcept{
		this->real /= rhs;
		this->imag /= rhs;
		return *this;
	}

	template<class TT>
	constexpr explicit operator Complex<TT>() const noexcept{ return Complex<TT>{(TT)real, (TT)imag}; }
	constexpr explicit operator T() const noexcept{ return real; }






	template<class TT> friend constexpr auto operator +(const Complex<TT> lhs, const Complex<TT> rhs) noexcept{ return Complex<TT>{lhs.real + rhs.real, lhs.imag + rhs.imag}; }
	template<class TT> friend constexpr auto operator -(const Complex<TT> lhs, const Complex<TT> rhs) noexcept{ return Complex<TT>{lhs.real - rhs.real, lhs.imag - rhs.imag}; }
	template<class TT> friend constexpr auto operator *(const Complex<TT> lhs, const Complex<TT> rhs) noexcept{ 
		return Complex<TT>{lhs.real*rhs.real - lhs.imag*rhs.imag, lhs.real*rhs.imag + lhs.imag*rhs.real};
	}
	template<class TT> friend auto operator /(const Complex<TT> lhs, const Complex<TT> rhs) noexcept{
		const TT divider = (TT)1 / (rhs.real*rhs.real + rhs.imag*rhs.imag);
		return Complex<TT>{(lhs.real*rhs.real + lhs.imag*rhs.imag) * divider, (lhs.imag*rhs.real - lhs.real*rhs.imag) * divider};
	}

	template<class TT> friend constexpr auto operator +(const Complex<TT> arg) noexcept{ return arg; }
	template<class TT> friend constexpr auto operator -(const Complex<TT> arg) noexcept{ return Complex<TT>{-arg.real, -arg.imag}; }


	template<class TT> friend constexpr auto operator +(const Complex<TT> lhs, const TT rhs) noexcept{ return Complex<TT>{lhs.real + rhs, lhs.imag}; }
	template<class TT> friend constexpr auto operator -(const Complex<TT> lhs, const TT rhs) noexcept{ return Complex<TT>{lhs.real - rhs, lhs.imag}; }
	template<class TT> friend constexpr auto operator *(const Complex<TT> lhs, const TT rhs) noexcept{ return Complex<TT>{lhs.real * rhs, lhs.imag * rhs}; }
	template<class TT> friend constexpr auto operator /(const Complex<TT> lhs, const TT rhs) noexcept{ return Complex<TT>{lhs.real / rhs, lhs.imag / rhs}; }

	template<class TT> friend constexpr auto operator +(const TT lhs, const Complex<TT> rhs) noexcept{ return Complex<TT>{lhs + rhs.real, rhs.imag}; }
	template<class TT> friend constexpr auto operator -(const TT lhs, const Complex<TT> rhs) noexcept{ return Complex<TT>{lhs - rhs.real, rhs.imag}; }
	template<class TT> friend constexpr auto operator *(const TT lhs, const Complex<TT> rhs) noexcept{ return Complex<TT>{lhs * rhs.real, lhs * rhs.imag}; }
	template<class TT> friend constexpr auto operator /(const TT lhs, const Complex<TT> rhs) noexcept{
		const TT multiplyer = lhs / (lhs.real*rhs.real + lhs.imag*rhs.imag);
		return Complex<TT>{rhs.real * multiplyer, -rhs.imag * multiplyer};
	}


	template<class TT> friend constexpr TT abs(const Complex<TT> arg) noexcept{ return sqrt(arg.real*arg.real + arg.imag*arg.imag); }
	template<class TT> friend constexpr TT absSquare(const Complex<TT> arg) noexcept{ return arg.real*arg.real + arg.imag*arg.imag; }
	template<class TT> friend constexpr TT arg(const Complex<TT> arg) noexcept{ return atan2(arg.imag, arg.real); }
	template<class TT> friend constexpr TT real(const Complex<TT> arg) noexcept{ return arg.real; }
	template<class TT> friend constexpr TT imag(const Complex<TT> arg) noexcept{ return arg.imag; }

	template<class TT> friend constexpr Complex<TT> conj(const Complex<TT> arg) noexcept{ return Complex<TT>{arg.real, -arg.imag}; }
	
	
	template<class TT>
	friend constexpr Complex<TT> exp(const Complex<TT> arg) noexcept{
		const TT modul = std::exp(arg.real);
		return Complex<TT>{modul*std::cos(arg.imag), modul*std::sin(arg.imag)};
	}
	template<class TT>
	friend constexpr Complex<TT> log(const Complex<TT> arg) noexcept{
		return Complex<TT>{std::log(absSquare(arg)) / (TT)2, std::atan2(arg.imag, arg.real)};
	}
	template<class TT>
	friend constexpr Complex<TT> pow(const Complex<TT> base, const Complex<TT> exponent) noexcept{
		return exp(log(base) * exponent);
	}
	template<class TT>
	friend constexpr Complex<TT> pow(const TT base, const Complex<TT> exponent) noexcept{
		const TT modul = std::pow(base, exponent.real);
		const TT angle = std::log(base) * exponent.imag;
		return Complex<TT>{modul*std::cos(angle), modul*std::sin(angle)};
	}
	template<class TT>
	friend constexpr Complex<TT> pow(const Complex<TT> base, const TT exponent) noexcept{
		const TT modul = std::pow(absSquare(base), exponent / (TT)2);
		const TT angle = std::atan2(base.imag, base.real) * exponent;
		return Complex<TT>{modul*std::cos(angle), modul*std::sin(angle)};
	}

	template<class TT>
	friend constexpr Complex<TT> sin(const Complex<TT> arg) noexcept{
		return Complex<TT>{std::sin(arg.real)*std::cosh(arg.imag), std::cos(arg.real)*std::sinh(arg.imag)};
	}
	template<class TT>
	friend constexpr Complex<TT> cos(const Complex<TT> arg) noexcept{
		return Complex<TT>{std::cos(arg.real)*std::cosh(arg.imag), -std::sin(arg.real)*std::sinh(arg.imag)};
	}
	template<class TT>
	friend constexpr Complex<TT> tan(const Complex<TT> arg) noexcept{
		const TT rCos = std::cos(arg.real);
		const TT iSinh = std::sinh(arg.imag);
		const TT denom = rCos*rCos + iSinh*iSinh;
		return Complex<TT>{std::sin(arg.real)*rCos/denom, iSinh*std::cosh(arg.imag)/denom};
	}
};





} // END OF NAMESPACE ///////////////////////////////////////