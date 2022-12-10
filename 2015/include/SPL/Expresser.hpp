#pragma once

#include "SPL/Arrays.hpp"
#include "SPL/Utils.hpp"
#include "SPL/Complex.hpp"



#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wnarrowing"

namespace sp{


namespace priv__{

union Fsdlfjiosdjfpowejhefjsdf{
	sp::DArray<uint64_t> data;

	Fsdlfjiosdjfpowejhefjsdf() : data() {}
	~Fsdlfjiosdjfpowejhefjsdf(){}
} PolynomialTempStorage;


} // END OF NAMESPACE PRIV


template<class Base> struct ExpresserExpr : Base {
	typedef typename std::decay_t<Base>::value_type valType;

	valType operator ()(const valType x) noexcept{
		valType result = (*this)[0];
		valType xPower = valType{1};
		for (size_t i=1; i<std::size(*this); ++i){
			xPower *= x;
			result += (*this)[i] * xPower;
		}
		return result;
	}
};


template<class Base = sp::DArray<double>>
struct Expresser : Base{
	typedef typename Base::value_type valType;
	static constexpr bool hasWrongDegree = false;

	template<class B>
	const Polynomial &operator =(const Polynomial<B> &rhs) noexcept{
		this->resize(std::size(rhs));
		std::copy(std::begin(rhs), std::end(rhs), std::begin(*this));
		return *this;
	}
	template<class B>
	const Polynomial &operator =(const PolynomialExpr<B> &rhs) noexcept{
		this->resize(std::size(rhs));
		for (size_t i=0; i!=std::size(*this); ++i)
			(*this)[i] = rhs[i];
		if (B::hasWrongDegree) repairDegree(*this);
		return *this;
	}


	template<class B>
	const Polynomial &operator +=(const Polynomial<B> &rhs) noexcept{
		this->resize(std::max(std::size(*this), std::size(rhs)));
		for (size_t i=0; i!=std::size(*this); ++i)
			(*this)[i] += rhs[i];
		repairDegree(*this);
		return *this;
	}
	template<class B>
	const Polynomial &operator -=(const Polynomial<B> &rhs) noexcept{
		this->resize(std::max(std::size(*this), std::size(rhs)));
		for (size_t i=0; i!=std::size(*this); ++i)
			(*this)[i] -= rhs[i];
		repairDegree(*this);
		return *this;
	}
	template<class B>
	const Polynomial &operator *=(const Polynomial<B> &rhs) noexcept{
		const size_t oldSize = std::size(*this);
		priv__::PolynomialTempStorage.data.expandBy(oldSize * ((sizeof(valType)+3) / 4));
		valType *const TempStorage = (valType *)std::end(priv__::PolynomialTempStorage.data) - oldSize - 1;
		std::move(std::begin(*this), std::end(*this), TempStorage);
		this->resize(oldSize + std::size(rhs) - 1);

		for (size_t i=0; i!=oldSize; ++i)
			for (size_t j=0; j!=std::size(rhs); ++j)
				(*this)[i+j] += TempStorage[i] * rhs[j];
		priv__::PolynomialTempStorage.data.shrinkBy(oldSize * ((sizeof(valType)+3) / 4));
		return *this;
	}

	template<class B>
	const Polynomial &operator +=(const PolynomialExpr<B> &rhs) noexcept{
		this->resize(std::max(std::size(*this), std::size(rhs)));
		for (size_t i=0; i!=std::size(*this); ++i)
			(*this)[i] += rhs[i];
		repairDegree(*this);
		return *this;
	}
	template<class B>
	const Polynomial &operator -=(const PolynomialExpr<B> &rhs) noexcept{
		this->resize(std::max(std::size(*this), std::size(rhs)));
		for (size_t i=0; i!=std::size(*this); ++i)
			(*this)[i] -= rhs[i];
		repairDegree(*this);
		return *this;
	}
	template<class B>
	const Polynomial &operator *=(const PolynomialExpr<B> &rhs) noexcept{
		const size_t oldSize = std::size(*this);
		priv__::PolynomialTempStorage.data.expandBy(oldSize * ((sizeof(valType)+3) / 4));
		valType *const TempStorage = (valType *)std::end(priv__::PolynomialTempStorage.data) - oldSize - 1;
		std::move(std::begin(*this), std::end(*this), TempStorage);
		this->resize(oldSize + std::size(rhs) - 1);

		for (size_t i=0; i!=oldSize; ++i)
			for (size_t j=0; j!=std::size(rhs); ++j)
				(*this)[i+j] += TempStorage[i] * rhs[j];
		if (B::hasWrongDegree) repairDegree(*this); 
		priv__::PolynomialTempStorage.data.shrinkBy(oldSize * ((sizeof(valType)+3) / 4));
		return *this;
	}


	valType operator ()(const valType x) const noexcept{
		valType result = (*this)[0];
		valType xPower = valType{1};
		for (size_t i=1; i<std::size(*this); ++i){
			xPower *= x;
			result += (*this)[i] * xPower;
		}
		return result;
	}
	
};


template<class Base>
void repairDegree(Polynomial<Base> &p) noexcept{
	size_t degr = std::size(p);
	do{ --degr; } while (p[degr]==(typename Base::value_type)0 && degr!=0);
	p.resize(degr + 1);
}

template<class Base>
size_t degree(const Polynomial<Base> &p) noexcept{ return std::size(p) - 1; }
template<class Base>
size_t degree(const PolynomialExpr<Base> &p) noexcept{ return std::size(p) - 1; }





template<class Arg1, class Arg2>
struct PolynomialExprAdd{
	Arg1 arg1;
	Arg2 arg2;

	typedef typename std::decay_t<Arg1>::value_type value_type;
	constexpr static bool hasWrongDegree = true;

	constexpr value_type operator [](const size_t index) const noexcept{ return arg1[index] + arg2[index]; }
	constexpr size_t size() const noexcept{ return std::max(std::size(arg1), std::size(arg2)); }
};
template<class Arg1, class Arg2>
struct PolynomialExprSubtract{
	Arg1 arg1;
	Arg2 arg2;

	typedef typename std::decay_t<Arg1>::value_type value_type;
	constexpr static bool hasWrongDegree = true;

	constexpr value_type operator [](const size_t index) const noexcept{ return arg1[index] - arg2[index]; }
	constexpr size_t size() const noexcept{ return std::max(std::size(arg1), std::size(arg2)); }
};
template<class Arg1, class Arg2>
struct PolynomialExprMultiply{
	Arg1 arg1;
	Arg2 arg2;

	typedef typename std::decay_t<Arg1>::value_type value_type;
	constexpr static bool hasWrongDegree = std::decay_t<Arg1>::hasWrongDegree || std::decay_t<Arg2>::hasWrongDegree;

	constexpr value_type operator [](const size_t index) const noexcept{
		value_type result = value_type{0};

		const ssize_t skipSize = index+1 - std::size(arg2);
		size_t i = skipSize<0 ? 0 : skipSize;
		size_t j = std::min(index, std::size(arg2)-1);
		for (; i!=std::size(arg1) && j!=(size_t)-1; ++i, --j)
			result += arg1[i] * arg2[j];

		return result;
	}
	constexpr size_t size() const noexcept{ return std::size(arg1) + std::size(arg2) - 1; }
};

template<class Arg>
struct PolynomialExprDifferentiate{
	Arg arg;

	typedef typename std::decay_t<Arg>::value_type value_type;
	constexpr static bool hasWrongDegree = Arg::hasWrongDegree;

	constexpr value_type operator [](const size_t index) const noexcept{
		return (value_type{index+1} * arg[index+1]);
	}

	constexpr size_t size() const noexcept{ return std::size(arg) - 1; }
};
template<class Arg>
struct PolynomialExprDifferentiate2{
	Arg arg;

	typedef typename std::decay_t<Arg>::value_type value_type;
	constexpr static bool hasWrongDegree = Arg::hasWrongDegree;

	constexpr value_type operator [](const size_t index) const noexcept{
		const auto temp = (value_type)(index + 1);
		return temp*(temp+1) * arg[index+2];
	}
	constexpr size_t size() const noexcept{ return std::size(arg) - 2; }
};
template<class Arg>
struct PolynomialExprIntegrate{
	Arg arg;

	typedef typename std::decay_t<Arg>::value_type value_type;
	constexpr static bool hasWrongDegree = Arg::hasWrongDegree;

	constexpr value_type operator [](const size_t index) const noexcept{
		if (!index) return (value_type)0;
		return arg[index-1] / (value_type)index;
	}
	constexpr size_t size() const noexcept{ return std::size(arg) + 1; }
};
template<class Arg>
struct PolynomialExprScalarMultiply{
	Arg arg;
	typename std::decay_t<Arg>::value_type scalar;

	typedef typename std::decay_t<Arg>::value_type value_type;
	constexpr static bool hasWrongDegree = false;

	constexpr value_type operator [](const size_t index) const noexcept{ return arg[index] * scalar; }
	constexpr size_t size() const noexcept{ return sp::choose(scalar==0, (size_t)1, std::size(arg)); }
};



template<class B1, class B2>
auto operator +(const Polynomial<B1> &lhs, const Polynomial<B2> &rhs) noexcept{
	return PolynomialExpr<PolynomialExprAdd<const Polynomial<B1> &, const Polynomial<B2> &>>{{lhs, rhs}};
}
template<class B1, class B2>
auto operator -(const Polynomial<B1> &lhs, const Polynomial<B2> &rhs) noexcept{
	return PolynomialExpr<PolynomialExprSubtract<const Polynomial<B1> &, const Polynomial<B2> &>>{{lhs, rhs}};
}
template<class B1, class B2>
auto operator *(const Polynomial<B1> &lhs, const Polynomial<B2> &rhs) noexcept{
	return PolynomialExpr<PolynomialExprMultiply<const Polynomial<B1> &, const Polynomial<B2> &>>{{lhs, rhs}};
}

template<class B1, class B2>
auto operator +(const PolynomialExpr<B1> &lhs, const Polynomial<B2> &rhs) noexcept{
	return PolynomialExpr<PolynomialExprAdd<PolynomialExpr<B1>, const Polynomial<B2> &>>{{lhs, rhs}};
}
template<class B1, class B2>
auto operator -(const PolynomialExpr<B1> &lhs, const Polynomial<B2> &rhs) noexcept{
	return PolynomialExpr<PolynomialExprSubtract<PolynomialExpr<B1>, const Polynomial<B2> &>>{{lhs, rhs}};
}
template<class B1, class B2>
auto operator *(const PolynomialExpr<B1> &lhs, const Polynomial<B2> &rhs) noexcept{
	return PolynomialExpr<PolynomialExprMultiply<PolynomialExpr<B1>, const Polynomial<B2> &>>{{lhs, rhs}};
}

template<class B1, class B2>
auto operator +(const Polynomial<B1> &lhs, const PolynomialExpr<B2> &rhs) noexcept{
	return PolynomialExpr<PolynomialExprAdd<const Polynomial<B1> &, PolynomialExpr<B2>>>{{lhs, rhs}};
}
template<class B1, class B2>
auto operator -(const Polynomial<B1> &lhs, const PolynomialExpr<B2> &rhs) noexcept{
	return PolynomialExpr<PolynomialExprSubtract<const Polynomial<B1> &, PolynomialExpr<B2>>>{{lhs, rhs}};
}
template<class B1, class B2>
auto operator *(const Polynomial<B1> &lhs, const PolynomialExpr<B2> &rhs) noexcept{
	return PolynomialExpr<PolynomialExprMultiply<const Polynomial<B1> &, PolynomialExpr<B2>>>{{lhs, rhs}};
}

template<class B1, class B2>
auto operator +(const PolynomialExpr<B1> &lhs, const PolynomialExpr<B2> &rhs) noexcept{
	return PolynomialExpr<PolynomialExprAdd<PolynomialExpr<B1>, PolynomialExpr<B2>>>{{lhs, rhs}};
}
template<class B1, class B2>
auto operator -(const PolynomialExpr<B1> &lhs, const PolynomialExpr<B2> &rhs) noexcept{
	return PolynomialExpr<PolynomialExprSubtract<PolynomialExpr<B1>, PolynomialExpr<B2>>>{{lhs, rhs}};
}

template<class B1, class B2>
auto operator *(const PolynomialExpr<B1> &lhs, const PolynomialExpr<B2> &rhs) noexcept{
	return PolynomialExpr<PolynomialExprMultiply<PolynomialExpr<B1>, PolynomialExpr<B2>>>{{lhs, rhs}};
}



template<class B>
auto diff(const Polynomial<B> &arg) noexcept{
	return PolynomialExpr<PolynomialExprDifferentiate<const Polynomial<B> &>>{{arg}};
}
template<class B>
auto diff2(const Polynomial<B> &arg) noexcept{
	return PolynomialExpr<PolynomialExprDifferentiate2<const Polynomial<B> &>>{{arg}};
}
template<class B>
auto integrate(const Polynomial<B> &arg) noexcept{
	return PolynomialExpr<PolynomialExprIntegrate<const Polynomial<B> &>>{{arg}};
}

template<class B>
auto diff(const PolynomialExpr<B> &arg) noexcept{
	return PolynomialExpr<PolynomialExprDifferentiate<PolynomialExpr<B>>>{{arg}};
}
template<class B>
auto diff2(const PolynomialExpr<B> &arg) noexcept{
	return PolynomialExpr<PolynomialExprDifferentiate2<PolynomialExpr<B>>>{{arg}};
}
template<class B>
auto integrate(const PolynomialExpr<B> &arg) noexcept{
	return PolynomialExpr<PolynomialExprIntegrate<PolynomialExpr<B>>>{{arg}};
}


template<class B>
auto operator *(const Polynomial<B> &lhs, const typename Polynomial<B>::valType &rhs) noexcept{
	return PolynomialExpr<PolynomialExprScalarMultiply<const Polynomial<B> &>>{{lhs, rhs}};
}

template<class B>
auto operator *(const typename Polynomial<B>::valType &lhs, const Polynomial<B> &rhs) noexcept{
	return PolynomialExpr<PolynomialExprScalarMultiply<const Polynomial<B> &>>{{rhs, lhs}};
}

template<class B>
auto operator *(const PolynomialExpr<B> &lhs, const typename PolynomialExpr<B>::valType &rhs) noexcept{
	return PolynomialExpr<PolynomialExprScalarMultiply<PolynomialExpr<B>>>{{lhs, rhs}};
}

template<class B>
auto operator *(const typename PolynomialExpr<B>::valType &lhs, const PolynomialExpr<B> &rhs) noexcept{
	return PolynomialExpr<PolynomialExprScalarMultiply<PolynomialExpr<B>>>{{rhs, lhs}};
}





template<class B1, class B2, class B3>
void divide(Polynomial<B1> &result, Polynomial<B2> &arg1, const Polynomial<B3> &arg2) noexcept{
	typedef typename B2::value_type valType;

	const size_t lastArg2Index = std::size(arg2) - 1;
	const size_t lastResultIndex = std::size(arg1) - std::size(arg2);
	if (lastResultIndex > std::numeric_limits<size_t>::max()/2){
		result.resize(1);
		result[0] = valType{};
		return;
	}
	result.resize(lastResultIndex + 1);

	const valType divider = arg2.back();
	for (size_t i=lastResultIndex; i!=(size_t)-1; --i){
		const valType res = arg1.back() / divider;
		arg1.pop_back();
		for (size_t j=lastArg2Index; j!=(size_t)-1; --j)
			arg1[i+j] -= res * arg2[j];
		result[i] = res;
	}
}
template<class Cont, class Base, size_t maxIterations = 100>
bool findRoots(Cont &results, const Polynomial<Base> &arg, const typename Base::value_type &precission) noexcept{
	// typedef typename Base::value_type valType;

	// results.resize(0);
	// const size_t requiredSize = 2 * std::size(arg) * ((sizeof(valType)+7) / 8);
	// priv__::PolynomialTempStorage.data.expandBy(requiredSize);
	
	// Polynomial<sp::View<valType>> pol1{{
	// 	(valType *)(std::end(priv__::PolynomialTempStorage.data) - requiredSize - 1),
	// 	(valType *)(std::end(priv__::PolynomialTempStorage.data) - requiredSize/2)
	// }};
	// Polynomial<sp::View<valType>> pol2{{
	// 	(valType *)(std::end(priv__::PolynomialTempStorage.data) - requiredSize/2),
	// 	(valType *)std::end(priv__::PolynomialTempStorage.data)
	// }};
	// Polynomial<sp::FiniteArray<valType, 2>> divider;
	// divider.resize(2);

	// pol1 = arg;
	// for (size_t i=0; i<degree(arg); ++i){
	// 	valType currX = pol1.back() / pol1.front();
	// 	valType prevX;

	// 	size_t iterCounter = 0;
	// 	do{
	// 		if (iterCounter == maxIterations){
	// 			priv__::PolynomialTempStorage.data.shrinkBy(requiredSize);
	// 			return true;
	// 		} ++iterCounter;
	// 		const valType polX = pol1(currX);
	// 		if (polX == valType{}) break;
	// 		prevX = currX;
	// 		currX -= pol1(currX) / diff(pol1)(currX);
	// 	} while (precission < abs(currX-prevX));
	// 	results.push_back(currX);

	// 	divider[0] = -currX;
	// 	divider[1] = (valType)1;

	// 	divide(pol2, pol1, divider);
	// 	std::swap(pol1, pol2);
	// }
	// priv__::PolynomialTempStorage.data.shrinkBy(requiredSize);
	// return false;

	typedef typename Base::value_type RealType;
	const size_t len = degree(arg);

	results.resize(len);
	{
		const float radius = (float)abs(
			RealType{len} * arg[0] / (arg[1] + arg[1])
		) + (float)abs(
			arg[len-1] / (RealType{2*len} * arg[len])
		);
		const float angle = (2.0*M_PI) / (float)len;

		float offset = angle * 0.25;
		for (auto &I : results){
			I = RealType{radius * cosf(offset)};
			offset += angle;
		}
	}


	size_t iterCounter = 0;
	RealType prevRoot;
	do{
		if (iterCounter == maxIterations) return true;
		++iterCounter;
		prevRoot = results[0];
		for (size_t j=0; j!=len; ++j){
			const RealType currRoot = results[j];
			const RealType argZ = arg(currRoot);

			RealType sum = RealType{0};
			for (size_t k=0; k!=j; ++k)
				sum += RealType{1} / (currRoot - results[k]);
			for (size_t k=j+1; k!=len; ++k)
				sum += RealType{1} / (currRoot - results[k]);

			results[j] -= argZ / (diff(arg)(currRoot) - argZ * sum);
		}
	} while (precission < abs(results[0] - prevRoot));
	return false;
}

template<class Cont, class Base, size_t maxIterations = 100>
bool findCRoots(Cont &results, const Polynomial<Base> &arg, const typename Base::value_type::value_type &precission) noexcept{
	typedef typename Base::value_type::valType RealType;
	typedef sp::Complex<RealType> ComplexType;
	const size_t len = degree(arg);

	results.resize(len);
	{
		const float radius = (float)abs(
			RealType{len} * arg[0] / (arg[1] + arg[1])
		) + (float)abs(
			arg[len-1] / (RealType{2*len} * arg[len])
		);
		const float angle = (2.0*M_PI) / (float)len;

		float offset = angle * 0.25;
		for (auto &I : results){
			I.real = RealType{radius * cosf(offset)};
			I.imag = RealType{radius * sinf(offset)};
			offset += angle;
		}
	}


	size_t iterCounter = 0;
	ComplexType prevRoot;
	do{
		if (iterCounter == maxIterations) return true;
		++iterCounter;
		prevRoot = results[0];
		for (size_t j=0; j!=len; ++j){
			const ComplexType currRoot = results[j];
			const ComplexType argZ = arg(currRoot);

			ComplexType sum = ComplexType{0};
			for (size_t k=0; k!=j; ++k)
				sum += ComplexType{1} / (currRoot - results[k]);
			for (size_t k=j+1; k!=len; ++k)
				sum += ComplexType{1} / (currRoot - results[k]);

			results[j] -= argZ / (diff(arg)(currRoot) - argZ * sum);
		}
	} while (precission < abs(results[0] - prevRoot));
	return false;
}

template<class B1, class B2, class B3>
void divide(Polynomial<B1> &result, Polynomial<B2> &arg1, const PolynomialExpr<B3> &arg2) noexcept{
	typedef typename B2::value_type valType;

	const size_t lastArg2Index = std::size(arg2) - 1;
	const size_t lastResultIndex = std::size(arg1) - std::size(arg2);
	if (lastResultIndex > std::numeric_limits<size_t>::max()/2){
		result.resize(1);
		result[0] = valType{};
		return;
	}
	result.resize(lastResultIndex + 1);

	const valType divider = arg2.back();
	for (size_t i=lastResultIndex; i!=(size_t)-1; --i){
		const valType res = arg1.back() / divider;
		arg1.pop_back();
		for (size_t j=lastArg2Index; j!=(size_t)-1; --j)
			arg1[i+j] -= res * arg2[j];
		result[i] = res;
	}
}
template<class Cont, class Base, size_t maxIterations = 100>
bool findRoots(Cont &results, const PolynomialExpr<Base> &arg, const typename Base::value_type &precission) noexcept{
	typedef typename Base::value_type valType;

	results.resize(0);
	const size_t requiredSize = 2 * std::size(arg) * ((sizeof(valType)+7) / 8);
	priv__::PolynomialTempStorage.data.expandBy(requiredSize);
	
	Polynomial<sp::View<valType>> pol1{{
		(valType *)(std::end(priv__::PolynomialTempStorage.data) - requiredSize - 1),
		(valType *)(std::end(priv__::PolynomialTempStorage.data) - requiredSize/2)
	}};
	Polynomial<sp::View<valType>> pol2{{
		(valType *)(std::end(priv__::PolynomialTempStorage.data) - requiredSize/2),
		(valType *)std::end(priv__::PolynomialTempStorage.data)
	}};
	Polynomial<sp::FiniteArray<valType, 2>> divider;
	divider.resize(2);

	pol1 = arg;
	for (size_t i=0; i<degree(arg); ++i){
		valType currX = pol1.back() / pol1.front();
		valType prevX;

		size_t iterCounter = 0;
		do{
			if (iterCounter == maxIterations){
				priv__::PolynomialTempStorage.data.shrinkBy(requiredSize);
				return true;
			} ++iterCounter;
			const valType polX = pol1(currX);
			if (polX == valType{}) break;
			prevX = currX;
			currX -= pol1(currX) / diff(pol1)(currX);
		} while (precission < abs(currX-prevX));
		results.push_back(currX);

		divider[0] = -currX;
		divider[1] = (valType)1;

		divideInplace(pol2, pol1, divider);
		std::swap(pol1, pol2);
	}
	priv__::PolynomialTempStorage.data.shrinkBy(requiredSize);
	return false;
}

#pragma GCC diagnostic pop

} // END OF NAMESPACE ///////////////////////////////////////////////////////////////////