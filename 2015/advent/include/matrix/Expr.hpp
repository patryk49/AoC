#pragma once

#include "Bases.hpp"

namespace sp{

template<SP_MATRIX_T(M)>
size_t rows(const M &arg) noexcept{ return arg.rows(); }
template<SP_MATRIX_T(M)>
size_t cols(const M &arg) noexcept{ return arg.cols(); }
template<SP_MATRIX_T(M)>
size_t size(const M &arg) noexcept{ return arg.size(); }
template<SP_MATRIX_T(M)>
void resize(M &arg, const size_t rows, const size_t cols) noexcept{ arg.resize(rows, cols); }

} namespace sp::priv__{


constexpr size_t CacheSize = 32768;
constexpr size_t CachePage = 64;

constexpr size_t CacheAvalible = CacheSize / 2;
constexpr size_t CacheBlockLen = intSqrt(CacheAvalible);


template<class Base> struct MatrixWrapper;



template<class Base>
struct MatrixWrapper : Base{

	template<SP_MATRIX_T(M)>
	const MatrixWrapper &operator =(M &&rhs) noexcept{
		this->resize(rhs.rows(), rhs.cols());
		if constexpr (std::decay_t<M>::undefMajor ? Base::rowMajor : std::decay_t<M>::rowMajor)
			for (size_t i=0; i!=this->rows(); ++i)
				for (size_t j=0; j!=this->cols(); ++j)
					(*this)(i, j) = rhs(i, j);
		else
			for (size_t i=0; i!=this->cols(); ++i)
				for (size_t j=0; j!=this->rows(); ++j)
					(*this)(j, i) = rhs(j, i);
		if constexpr (std::decay_t<M>::usesBuffer) MatrixTempStorage.data.resize(MatrixTempStorage.stackSize);
		return *this;
	}

	template<SP_MATRIX_T(M)>
	const MatrixWrapper &operator +=(M &&rhs) noexcept{
		for (size_t i=0; i!=this->rows(); ++i)
			for (size_t j=0; j!=this->cols(); ++j)
				(*this)(i, j) += rhs(i, j);
		return *this;
	}
	template<SP_MATRIX_T(M)>
	const MatrixWrapper &operator -=(M &&rhs) noexcept{
		for (size_t i=0; i!=this->rows(); ++i)
			for (size_t j=0; j!=this->cols(); ++j)
				(*this)(i, j) -= rhs(i, j);
		return *this;
	}
	template<SP_MATRIX_T(M)>
	const MatrixWrapper &operator *=(M &&rhs) noexcept{
		this->resize(rhs.cols(), rhs.rows());

		const size_t oldSize = std::size(MatrixTempStorage.data);
		MatrixTempStorage.data.expandBy((std::size(*this) * sizeof(typename Base::valType) + 7) / 8);
		typename Base::valType *const TempStorage = (typename Base::valType *)std::begin(MatrixTempStorage.data) + oldSize;
		
		std::move(std::begin(*this), std::end(*this), TempStorage);
		if constexpr (Base::rowMajor){
			for (size_t i=0; i!=this->cols(); ++i)
				for (size_t j=0; j!=rhs.rows(); ++j)
					for (size_t k=0; k!=this->rows(); ++k)
						(*this)(i, j) = TempStorage[i*this->cols()+k] * rhs(j, k);
		} else{
			for (size_t i=0; i!=this->cols(); ++i)
				for (size_t j=0; j!=rhs.rows(); ++j)
					for (size_t k=0; k!=this->rows(); ++k)
						(*this)(i, j) = TempStorage[i+k*this->cols()] * rhs(j, k);
		}
			
		MatrixTempStorage.data.resize(oldSize);
		return *this;
	}
	
};


template<class M, bool isLVal>
struct MatrixExprTranspose{
	static constexpr StupidMatrixFlagType MatrixFlag{};
	M arg;

	typedef std::remove_reference_t<M> Arg;

	typedef typename Arg::valType valType;
	constexpr static bool rowMajor = !Arg::rowMajor;
	constexpr static bool undefMajor = Arg::undefMajor;
	constexpr static bool usesBuffer = Arg::usesBuffer;

	[[nodiscard]] constexpr size_t rows() const noexcept{ return arg.cols(); }
	[[nodiscard]] constexpr size_t cols() const noexcept{ return arg.rows(); }
	[[nodiscard]] constexpr size_t size() const noexcept{ return arg.size(); }
	[[nodiscard]] constexpr size_t capacity() const noexcept{ return arg.capacity(); }

	constexpr void resize(const size_t r, const size_t c) noexcept{ arg.resize(c, r); };

	constexpr std::conditional_t<isLVal, valType &, valType>
		operator ()(const size_t r, const size_t c) noexcept{ return arg(c, r); }
	[[nodiscard]] constexpr valType operator ()(const size_t r, const size_t c) const noexcept{ return arg(c, r); }
};
template<class M, class Cont, bool isLVal>
struct MatrixExprPermuteRows{
	static constexpr StupidMatrixFlagType MatrixFlag{};
	M arg;
	const Cont *permuts;

	typedef std::remove_reference_t<M> Arg;

	typedef typename Arg::valType valType;
	constexpr static bool isExpr = Arg::isExpr;
	constexpr static bool rowMajor = !Arg::rowMajor;
	constexpr static bool undefMajor = Arg::undefMajor;
	constexpr static bool usesBuffer = Arg::usesBuffer;

	[[nodiscard]] constexpr size_t rows() const noexcept{ return arg.cols(); }
	[[nodiscard]] constexpr size_t cols() const noexcept{ return arg.rows(); }
	[[nodiscard]] constexpr size_t size() const noexcept{ return arg.size(); }
	[[nodiscard]] constexpr size_t capacity() const noexcept{ return arg.capacity(); }

	constexpr void resize(const size_t r, const size_t c) noexcept{ arg.resize(c, r); };

	constexpr std::conditional_t<isLVal, valType &, valType>
		operator ()(const size_t r, const size_t c) noexcept{ return arg((*permuts)[r], c); }
	[[nodiscard]] constexpr valType operator ()(const size_t r, const size_t c) const noexcept{ return arg((*permuts)[r], c); }
};
template<class M, class Cont, bool isLVal>
struct MatrixExprPermuteCols{
	static constexpr StupidMatrixFlagType MatrixFlag{};
	M arg;
	const Cont *permuts;

	typedef std::remove_reference_t<M> Arg;

	typedef typename Arg::valType valType;
	constexpr static bool isExpr = Arg::isExpr;
	constexpr static bool rowMajor = !Arg::rowMajor;
	constexpr static bool undefMajor = Arg::undefMajor;
	constexpr static bool usesBuffer = Arg::usesBuffer;

	[[nodiscard]] constexpr size_t rows() const noexcept{ return arg.cols(); }
	[[nodiscard]] constexpr size_t cols() const noexcept{ return arg.rows(); }
	[[nodiscard]] constexpr size_t size() const noexcept{ return arg.size(); }
	[[nodiscard]] constexpr size_t capacity() const noexcept{ return arg.capacity(); }

	constexpr void resize(const size_t r, const size_t c) noexcept{ arg.resize(c, r); };

	constexpr std::conditional_t<isLVal, valType &, valType>
		operator ()(const size_t r, const size_t c) noexcept{ return arg(r, (*permuts)[c]); }
	[[nodiscard]] constexpr valType operator ()(const size_t r, const size_t c) const noexcept{ return arg(r, (*permuts)[c]); }
};


template<class M, auto operation>
struct MatrixExprElStatUnaryOp{
	static constexpr StupidMatrixFlagType MatrixFlag{};
	M arg;

	typedef std::remove_reference_t<M> Arg;

	typedef typename Arg::valType valType;
	constexpr static bool rowMajor = Arg::rowMajor;
	constexpr static bool undefMajor = Arg::undefMajor;
	constexpr static bool usesBuffer = Arg::usesBuffer;

	[[nodiscard]] constexpr size_t rows() const noexcept{ return arg.rows(); }
	[[nodiscard]] constexpr size_t cols() const noexcept{ return arg.cols(); }
	[[nodiscard]] constexpr size_t size() const noexcept{ return arg.size(); }
	[[nodiscard]] constexpr size_t capacity() const noexcept{ return 0; }

	[[nodiscard]] constexpr valType operator ()(const size_t r, const size_t c) const noexcept{ return operation(arg(r, c)); }
};
template<class M, class Operation>
struct MatrixExprElDynUnaryOp{
	static constexpr StupidMatrixFlagType MatrixFlag{};
	M arg;
	Operation operation;


	typedef std::remove_reference_t<M> Arg;

	typedef typename Arg::valType valType;
	constexpr static bool rowMajor = Arg::rowMajor;
	constexpr static bool undefMajor = Arg::undefMajor;
	constexpr static bool usesBuffer = Arg::usesBuffer;

	[[nodiscard]] constexpr size_t rows() const noexcept{ return arg.rows(); }
	[[nodiscard]] constexpr size_t cols() const noexcept{ return arg.cols(); }
	[[nodiscard]] constexpr size_t size() const noexcept{ return arg.size(); }
	[[nodiscard]] constexpr size_t capacity() const noexcept{ return 0; }

	[[nodiscard]] constexpr valType operator ()(const size_t r, const size_t c) const noexcept{ return operation(arg(r, c)); }
};

template<class T, bool rowMaj>
struct MatrixExprCopy{
	static constexpr StupidMatrixFlagType MatrixFlag{};
	size_t startIndex;
	uint32_t rowsNumber;
	uint32_t colsNumber;

	template<SP_MATRIX_T(M)>
	MatrixExprCopy(M &&A) noexcept : startIndex(std::size(MatrixTempStorage.data)), rowsNumber(A.rows()), colsNumber(A.cols()){
		MatrixTempStorage.data.expandBy((this->size() * sizeof(T) + 7) / 8);
		T *I = (T *)MatrixTempStorage.data.begin() + startIndex;
		if constexpr (rowMaj)
			for (size_t i=0; i!=rowsNumber; ++i)
				for (size_t j=0; j!=colsNumber; ++j, ++I)
					*I = A(i, j);
		else
			for (size_t i=0; i!=colsNumber; ++i)
				for (size_t j=0; j!=rowsNumber; ++j, ++I)
					*I = A(j, i);
	}


	typedef T valType;
	constexpr static bool rowMajor = rowMaj;
	constexpr static bool undefMajor = false;
	constexpr static bool usesBuffer = true;

	[[nodiscard]] constexpr size_t rows() const noexcept{ return rowsNumber; }
	[[nodiscard]] constexpr size_t cols() const noexcept{ return colsNumber; }
	[[nodiscard]] constexpr size_t size() const noexcept{ return rowsNumber * colsNumber; }
	[[nodiscard]] constexpr size_t capacity() const noexcept{ return 0; }

	[[nodiscard]] constexpr T operator ()(const size_t r, const size_t c) const noexcept{
		if constexpr (rowMajor)
			return *((T *)std::begin(MatrixTempStorage.data)+startIndex + r*(size_t)colsNumber + c);
		else
			return *((T *)std::begin(MatrixTempStorage.data)+startIndex + r + c*(size_t)rowsNumber);
	}
};



template<class M1, class M2>
struct MatrixExprAdd{
	static constexpr StupidMatrixFlagType MatrixFlag{};
	M1 arg1;
	M2 arg2;

	typedef std::remove_reference_t<M1> Arg1;
	typedef std::remove_reference_t<M2> Arg2;

	typedef typename Arg1::valType valType;
	constexpr static bool rowMajor = Arg1::rowMajor;
	constexpr static bool undefMajor = Arg1::undefMajor || Arg2::undefMajor || Arg1::rowMajor==Arg2::rowMajor;
	constexpr static bool usesBuffer = Arg1::usesBuffer || Arg2::usesBuffer;

	[[nodiscard]] constexpr size_t rows() const noexcept{ return arg1.rows(); }
	[[nodiscard]] constexpr size_t cols() const noexcept{ return arg1.cols(); }
	[[nodiscard]] constexpr size_t size() const noexcept{ return arg1.size(); }
	[[nodiscard]] constexpr size_t capacity() const noexcept{ return 0; }

	[[nodiscard]] constexpr valType operator ()(const size_t r, const size_t c) const noexcept{ return arg1(r, c) + arg2(r, c); }
};
template<class M1, class M2>
struct MatrixExprSubtract{
	static constexpr StupidMatrixFlagType MatrixFlag{};
	M1 arg1;
	M2 arg2;

	typedef std::remove_reference_t<M1> Arg1;
	typedef std::remove_reference_t<M2> Arg2;

	typedef typename Arg1::valType valType;
	constexpr static bool rowMajor = Arg1::rowMajor;
	constexpr static bool undefMajor = Arg1::undefMajor || Arg2::undefMajor || Arg1::rowMajor==Arg2::rowMajor;
	constexpr static bool usesBuffer = Arg1::usesBuffer || Arg2::usesBuffer;

	[[nodiscard]] constexpr size_t rows() const noexcept{ return arg1.rows(); }
	[[nodiscard]] constexpr size_t cols() const noexcept{ return arg1.cols(); }
	[[nodiscard]] constexpr size_t size() const noexcept{ return arg1.size(); }
	[[nodiscard]] constexpr size_t capacity() const noexcept{ return 0; }

	[[nodiscard]] constexpr valType operator ()(const size_t r, const size_t c) const noexcept{ return arg1(r, c) - arg2(r, c); }
};
template<class M1, class M2>
struct MatrixExprElMul{
	static constexpr StupidMatrixFlagType MatrixFlag{};
	M1 arg1;
	M2 arg2;

	typedef std::remove_reference_t<M1> Arg1;
	typedef std::remove_reference_t<M2> Arg2;

	typedef typename Arg1::valType valType;
	constexpr static bool rowMajor = Arg1::rowMajor;
	constexpr static bool undefMajor = Arg1::undefMajor || Arg2::undefMajor || Arg1::rowMajor==Arg2::rowMajor;
	constexpr static bool usesBuffer = Arg1::usesBuffer || Arg2::usesBuffer;

	[[nodiscard]] constexpr size_t rows() const noexcept{ return arg1.rows(); }
	[[nodiscard]] constexpr size_t cols() const noexcept{ return arg1.cols(); }
	[[nodiscard]] constexpr size_t size() const noexcept{ return arg1.size(); }
	[[nodiscard]] constexpr size_t capacity() const noexcept{ return 0; }

	[[nodiscard]] constexpr valType operator ()(const size_t r, const size_t c) const noexcept{ return arg1(r, c) * arg2(r, c); }
};
template<class M1, class M2>
struct MatrixExprElDiv{
	static constexpr StupidMatrixFlagType MatrixFlag{};
	M1 arg1;
	M2 arg2;

	typedef std::remove_reference_t<M1> Arg1;
	typedef std::remove_reference_t<M2> Arg2;

	typedef typename Arg1::valType valType;
	constexpr static bool rowMajor = Arg1::rowMajor;
	constexpr static bool undefMajor = Arg1::undefMajor || Arg2::undefMajor || Arg1::rowMajor==Arg2::rowMajor;
	constexpr static bool usesBuffer = Arg1::usesBuffer || Arg2::usesBuffer;

	[[nodiscard]] constexpr size_t rows() const noexcept{ return arg1.rows(); }
	[[nodiscard]] constexpr size_t cols() const noexcept{ return arg1.cols(); }
	[[nodiscard]] constexpr size_t size() const noexcept{ return arg1.size(); }
	[[nodiscard]] constexpr size_t capacity() const noexcept{ return 0; }

	[[nodiscard]] constexpr valType operator ()(const size_t r, const size_t c) const noexcept{ return arg1(r, c) / arg2(r, c); }
};
template<class M1, class M2, auto operation>
struct MatrixExprElStatOp{
	static constexpr StupidMatrixFlagType MatrixFlag{};
	M1 arg1;
	M2 arg2;


	typedef std::remove_reference_t<M1> Arg1;
	typedef std::remove_reference_t<M2> Arg2;

	typedef typename Arg1::valType valType;
	constexpr static bool rowMajor = Arg1::rowMajor;
	constexpr static bool undefMajor = Arg1::undefMajor || Arg2::undefMajor || Arg1::rowMajor==Arg2::rowMajor;
	constexpr static bool usesBuffer = Arg1::usesBuffer || Arg2::usesBuffer;

	[[nodiscard]] constexpr size_t rows() const noexcept{ return arg1.rows(); }
	[[nodiscard]] constexpr size_t cols() const noexcept{ return arg1.cols(); }
	[[nodiscard]] constexpr size_t size() const noexcept{ return arg1.size(); }
	[[nodiscard]] constexpr size_t capacity() const noexcept{ return 0; }

	[[nodiscard]] constexpr valType operator ()(const size_t r, const size_t c) const noexcept{ return operation(arg1(r, c), arg2(r, c)); }
};
template<class M1, class M2, class Operation>
struct MatrixExprElDynOp{
	static constexpr StupidMatrixFlagType MatrixFlag{};
	M1 arg1;
	M2 arg2;
	Operation operation;


	typedef std::remove_reference_t<M1> Arg1;
	typedef std::remove_reference_t<M2> Arg2;

	typedef typename Arg1::valType valType;
	constexpr static bool rowMajor = Arg1::rowMajor;
	constexpr static bool undefMajor = Arg1::undefMajor || Arg2::undefMajor || Arg1::rowMajor==Arg2::rowMajor;
	constexpr static bool usesBuffer = Arg1::usesBuffer || Arg2::usesBuffer;

	[[nodiscard]] constexpr size_t rows() const noexcept{ return arg1.rows(); }
	[[nodiscard]] constexpr size_t cols() const noexcept{ return arg1.cols(); }
	[[nodiscard]] constexpr size_t size() const noexcept{ return arg1.size(); }
	[[nodiscard]] constexpr size_t capacity() const noexcept{ return 0; }

	[[nodiscard]] constexpr valType operator ()(const size_t r, const size_t c) const noexcept{ return operation(arg1(r, c), arg2(r, c)); }
};
template<class M1, class M2>
struct MatrixExprMultiply{
	static constexpr StupidMatrixFlagType MatrixFlag{};
	M1 arg1;
	M2 arg2;

	typedef std::remove_reference_t<M1> Arg1;
	typedef std::remove_reference_t<M2> Arg2;

	typedef typename Arg1::valType valType;
	constexpr static bool rowMajor = false;
	constexpr static bool undefMajor = true;
	constexpr static bool usesBuffer = Arg1::usesBuffer || Arg2::usesBuffer;

	[[nodiscard]] constexpr size_t rows() const noexcept{ return arg1.rows(); }
	[[nodiscard]] constexpr size_t cols() const noexcept{ return arg2.cols(); }
	[[nodiscard]] constexpr size_t size() const noexcept{ return arg1.rows() * arg2.cols(); }
	[[nodiscard]] constexpr size_t capacity() const noexcept{ return 0; }

	[[nodiscard]] constexpr valType operator ()(const size_t r, const size_t c) const noexcept{
		valType res = (valType)0;
		for (size_t i=0; i!=arg1.cols(); ++i)
			res += arg1(r, i) * arg2(i, c);
		return res;
	}
};
template<class M>
struct MatrixExprScalarMultiply{
	static constexpr StupidMatrixFlagType MatrixFlag{};
	typedef std::remove_reference_t<M> Arg;
	typedef typename Arg::valType valType;

	M arg1;
	valType arg2;

	constexpr static bool rowMajor = Arg::rowMajor;
	constexpr static bool undefMajor = Arg::undefMajor;
	constexpr static bool usesBuffer = Arg::usesBuffer;

	[[nodiscard]] constexpr size_t rows() const noexcept{ return arg1.rows(); }
	[[nodiscard]] constexpr size_t cols() const noexcept{ return arg1.cols(); }
	[[nodiscard]] constexpr size_t size() const noexcept{ return arg1.size(); }
	[[nodiscard]] constexpr size_t capacity() const noexcept{ return 0; }

	[[nodiscard]] constexpr valType operator ()(const size_t r, const size_t c) const noexcept{ return arg1(r, c) * arg2; }
};





template<auto operation>
struct MatrixExprStatGenerator{
	static constexpr StupidMatrixFlagType MatrixFlag{};
	uint32_t rowsNumber;
	uint32_t colsNumber;

	typedef decltype(operation(0, 0)) valType;
	constexpr static bool rowMajor = true;
	constexpr static bool undefMajor = true;
	constexpr static bool usesBuffer = false;

	[[nodiscard]] constexpr size_t rows() const noexcept{ return rowsNumber; }
	[[nodiscard]] constexpr size_t cols() const noexcept{ return colsNumber; }
	[[nodiscard]] constexpr size_t size() const noexcept{ return rowsNumber * colsNumber; }
	[[nodiscard]] constexpr size_t capacity() const noexcept{ return 0; }

	[[nodiscard]] constexpr valType operator ()(const size_t r, const size_t c) const noexcept{ return operation(r, c); }
};
template<class Operation>
struct MatrixExprDynGenerator{
	static constexpr StupidMatrixFlagType MatrixFlag{};
	uint32_t rowsNumber;
	uint32_t colsNumber;
	Operation operation;

	typedef decltype(operation(0, 0)) valType;
	constexpr static bool rowMajor = true;
	constexpr static bool undefMajor = true;
	constexpr static bool usesBuffer = false;

	[[nodiscard]] constexpr size_t rows() const noexcept{ return rowsNumber; }
	[[nodiscard]] constexpr size_t cols() const noexcept{ return colsNumber; }
	[[nodiscard]] constexpr size_t size() const noexcept{ return rowsNumber * colsNumber; }
	[[nodiscard]] constexpr size_t capacity() const noexcept{ return 0; }

	[[nodiscard]] constexpr valType operator ()(const size_t r, const size_t c) const noexcept{ return operation(r, c); }
};
template<class T>
struct MatrixExprUniformValue{
	static constexpr StupidMatrixFlagType MatrixFlag{};
	uint32_t rowsNumber;
	uint32_t colsNumber;
	T value;

	typedef T valType;
	constexpr static bool rowMajor = true;
	constexpr static bool undefMajor = true;
	constexpr static bool usesBuffer = false;

	[[nodiscard]] constexpr size_t rows() const noexcept{ return rowsNumber; }
	[[nodiscard]] constexpr size_t cols() const noexcept{ return colsNumber; }
	[[nodiscard]] constexpr size_t size() const noexcept{ return rowsNumber * colsNumber; }
	[[nodiscard]] constexpr size_t capacity() const noexcept{ return 0; }

	[[nodiscard]] constexpr valType operator ()(const size_t r, const size_t c) const noexcept{ return value; }
};



} namespace sp{
using namespace priv__;
 



template<SP_MATRIX_T(M)>
[[nodiscard]] auto tr(M &&arg) noexcept{
	return MatrixExprTranspose<CRemRRef<M>, false>{arg};
}
template<SP_MATRIX_T(M)>
auto ltr(M &&arg) noexcept{
	return MatrixWrapper<MatrixExprTranspose<RemRRef<M>, true>>{{arg}};
}


template<SP_MATRIX_T(M), class Cont>
[[nodiscard]] auto permRows(M &&arg, const Cont &permuts) noexcept{
	SP_MATRIX_ERROR(rows(arg) != std::size(permuts), "permutation array must have the same length as number of permuted matrix's rows");
	return MatrixExprPermuteRows<CRemRRef<M>, Cont, false>{arg, &permuts};
}
template<SP_MATRIX_T(M), class Cont>
auto lpermRows(M &&arg, const Cont &permuts) noexcept{
	SP_MATRIX_ERROR(rows(arg) != std::size(permuts), "permutation array must have the same length as number of permuted matrix's rows");
	return MatrixWrapper<MatrixExprPermuteRows<RemRRef<M>, Cont, true>>{{arg, &permuts}};
}

template<SP_MATRIX_T(M), class Cont>
[[nodiscard]] auto permCols(M &&arg, const Cont &permuts) noexcept{
	SP_MATRIX_ERROR(cols(arg) != std::size(permuts), "permutation array must have the same length as number of permuted matrix's columns");
	return MatrixExprPermuteCols<CRemRRef<M>, Cont, false>{arg, &permuts};
}
template<SP_MATRIX_T(M), class Cont>
auto lpermCols(M &&arg, const Cont &permuts) noexcept{
	SP_MATRIX_ERROR(cols(arg) != std::size(permuts), "permutation array must have the same length as number of permuted matrix's columns");
	return MatrixWrapper<MatrixExprPermuteCols<RemRRef<M>, Cont, true>>{{arg, &permuts}};
}

template<SP_MATRIX_T(M)>
[[nodiscard]] auto cp(const M &arg) noexcept{ return MatrixExprCopy<typename std::decay_t<M>::valType, M::rowMajor>{arg}; }

template<auto operation, SP_MATRIX_T(M)>
[[nodiscard]] auto apply(M &&arg) noexcept{
	return MatrixExprElStatUnaryOp<CRemRRef<M>, operation>{arg};
}

template<SP_MATRIX_T(M), class Operation>
[[nodiscard]] auto apply(M &&arg, Operation &&operation) noexcept{
	return MatrixExprElDynUnaryOp<CRemRRef<M>, decltype(std::forward<Operation>(operation))>{arg, std::forward<Operation>(operation)};
}

template<SP_MATRIX_T(M1), SP_MATRIX_T(M2)>
[[nodiscard]] auto elwiseMul(M1 &&lhs, M2 &&rhs) noexcept{
	SP_MATRIX_ERROR(rows(lhs)!=rows(rhs) || cols(lhs)!=cols(rhs), "element wise multiplied matrices cannot have different dimensions");
	return MatrixExprElMul<CRemRRef<M1>, CRemRRef<M2>>{lhs, rhs};
}

template<SP_MATRIX_T(M1), SP_MATRIX_T(M2)>
[[nodiscard]] auto elwiseDiv(M1 &&lhs, M2 &&rhs) noexcept{
	SP_MATRIX_ERROR(rows(lhs)!=rows(rhs) || cols(lhs)!=cols(rhs), "element wise divided matrices cannot have different dimensions");
	return MatrixExprElDiv<CRemRRef<M1>, CRemRRef<M2>>{lhs, rhs};
}

template<SP_MATRIX_T(M1), SP_MATRIX_T(M2), class Operation>
[[nodiscard]] auto apply(M1 &&lhs, M2 &&rhs, Operation &&operation) noexcept{
	SP_MATRIX_ERROR(rows(lhs)!=rows(rhs) || cols(lhs)!=cols(rhs), "element wise operated matrices cannot have different dimensions");
	return MatrixExprElDynOp<CRemRRef<M1>, CRemRRef<M2>, decltype(std::forward<Operation>(operation))>{lhs, rhs, std::forward<Operation>(operation)};
}

template<auto operation, SP_MATRIX_T(M1), SP_MATRIX_T(M2)>
[[nodiscard]] auto apply(M1 &&lhs, M2 &&rhs) noexcept{
	SP_MATRIX_ERROR(rows(lhs)!=rows(rhs) || cols(lhs)!=cols(rhs), "element wise operated matrices cannot have different dimensions");
	return MatrixExprElStatOp<CRemRRef<M1>, CRemRRef<M2>, operation>{lhs, rhs};
}






template<auto operation>
[[nodiscard]] auto generate(const uint32_t rowsNumber, const uint32_t colsNumber) noexcept{
	return MatrixExprStatGenerator<operation>{rowsNumber, colsNumber};
}
template<class Operation>
[[nodiscard]] auto generate(const uint32_t rowsNumber, const uint32_t colsNumber, Operation &&operation) noexcept{
	return MatrixExprDynGenerator<decltype(std::forward<Operation>(operation))>{rowsNumber, colsNumber, std::forward<Operation>(operation)};
}
template<class T>
[[nodiscard]] auto uniform(const uint32_t rowsNumber, const uint32_t colsNumber, T value) noexcept{
	return MatrixExprUniformValue<T>{rowsNumber, colsNumber, value};
}


} namespace{
using namespace sp;
using namespace sp::priv__;

template<SP_MATRIX_T(M1), SP_MATRIX_T(M2)>
[[nodiscard]] auto operator +(M1 &&lhs, M2 &&rhs) noexcept{
	SP_MATRIX_ERROR(rows(lhs)!=rows(rhs) || cols(lhs)!=cols(rhs), "added matrices cannot have different dimensions");
	return MatrixExprAdd<CRemRRef<M1>, CRemRRef<M2>>{lhs, rhs};	
}

template<SP_MATRIX_T(M1), SP_MATRIX_T(M2)>
[[nodiscard]] auto operator -(M1 &&lhs, M2 &&rhs) noexcept{
	SP_MATRIX_ERROR(rows(lhs)!=rows(rhs) || cols(lhs)!=cols(rhs), "subtracted matrices cannot have different dimensions");
	return MatrixExprSubtract<CRemRRef<M1>, CRemRRef<M2>>{lhs, rhs};	
}

template<SP_MATRIX_T(M1), SP_MATRIX_T(M2)>
[[nodiscard]] auto operator *(M1 &&lhs, M2 &&rhs) noexcept{
	SP_MATRIX_ERROR(cols(lhs) != rows(rhs), "multiplied matrices have wrong dimensionss");
	return MatrixExprMultiply<CRemRRef<M1>, CRemRRef<M2>>{lhs, rhs};
}



template<SP_MATRIX_T(M)>
[[nodiscard]] auto operator *(M &&lhs, const typename std::decay_t<M>::valType &rhs) noexcept{
	return MatrixExprScalarMultiply<CRemRRef<M>>{lhs, rhs};
}
template<SP_MATRIX_T(M)>
[[nodiscard]] auto operator *(const typename std::decay_t<M>::valType &lhs, M &&rhs) noexcept{
	return MatrixExprScalarMultiply<CRemRRef<M>>{rhs, lhs};
}

template<SP_MATRIX_T(M)>
[[nodiscard]] auto operator /(M &&lhs, const typename std::decay_t<M>::valType &rhs) noexcept{
	return MatrixExprScalarMultiply<CRemRRef<M>>{lhs, (typename std::decay_t<M>::valType)1 / rhs};
}

} // END OF OPERATORS















namespace sp{

template<SP_VECTOR_T(V)>
size_t size(V &&arg) noexcept{ return arg.size(); }
template<SP_VECTOR_T(V)>
void resize(V &&arg, const size_t n) noexcept{ arg.resize(n); }

} namespace sp::priv__{

template<class Base> struct VectorWrapper;


template<class Base>
struct VectorWrapper : Base{

	template<SP_VECTOR_T(V)>
	const VectorWrapper &operator =(V &&rhs) noexcept{
		this->resize(rhs.size());
		for (size_t i=0; i!=this->size(); ++i)
			(*this)[i] = rhs[i];
		return *this;
	}

	template<SP_VECTOR_T(V)>
	const VectorWrapper &operator +=(V &&rhs) noexcept{
		for (size_t i=0; i!=this->size(); ++i)
			(*this)[i] += rhs[i];
		return *this;
	}
	template<SP_VECTOR_T(V)>
	const VectorWrapper &operator -=(V &&rhs) noexcept{
		for (size_t i=0; i!=this->size(); ++i)
			(*this)[i] -= rhs[i];
		return *this;
	}
	
};


template<class M, class Cont, bool isLVal>
struct VectorExprPermute{
	static constexpr StupidVectorFlagType VectorFlag{};
	M arg;
	const Cont *permuts;

	typedef std::remove_reference_t<M> Arg;

	typedef typename Arg::valType valType;
	constexpr static bool isExpr = Arg::isExpr;
	constexpr static bool usesBuffer = Arg::usesBuffer;

	[[nodiscard]] constexpr size_t size() const noexcept{ return arg.size(); }
	[[nodiscard]] constexpr size_t capacity() const noexcept{ return arg.capacity(); }

	constexpr void resize(const size_t n) noexcept{ arg.resize(n); };

	constexpr std::conditional_t<isLVal, valType &, valType>
		operator [](const size_t i) noexcept{ return arg[(*permuts)[i]]; }
	[[nodiscard]] constexpr valType operator [](const size_t i) const noexcept{ return arg[(*permuts)[i]]; }
};


template<class M, auto operation>
struct VectorExprElStatUnaryOp{
	static constexpr StupidVectorFlagType VectorFlag{};
	M arg;

	typedef std::remove_reference_t<M> Arg;

	typedef typename Arg::valType valType;
	constexpr static bool usesBuffer = Arg::usesBuffer;

	[[nodiscard]] constexpr size_t size() const noexcept{ return arg.size(); }
	[[nodiscard]] constexpr size_t capacity() const noexcept{ return 0; }

	[[nodiscard]] constexpr valType operator [](const size_t i) const noexcept{ return operation(arg[i]); }
};
template<class M, class Operation>
struct VectorExprElDynUnaryOp{
	static constexpr StupidVectorFlagType VectorFlag{};
	M arg;
	Operation operation;

	typedef std::remove_reference_t<M> Arg;

	typedef typename Arg::valType valType;
	constexpr static bool usesBuffer = Arg::usesBuffer;

	[[nodiscard]] constexpr size_t size() const noexcept{ return arg.size(); }
	[[nodiscard]] constexpr size_t capacity() const noexcept{ return 0; }

	[[nodiscard]] constexpr valType operator [](const size_t i) const noexcept{ return operation(arg[i]); }
};

template<class T>
struct VectorExprCopy{
	static constexpr StupidVectorFlagType VectorFlag{};
	size_t startIndex;
	uint32_t len;

	template<SP_VECTOR_T(V)>
	VectorExprCopy(V &&A) noexcept : startIndex(std::size(MatrixTempStorage.data)), len(A.size()){
		MatrixTempStorage.data.expandBy((this->size() * sizeof(T) + 7) / 8);
		T *I = (T *)MatrixTempStorage.data.begin() + startIndex;
		for (size_t i=0; i!=len; ++i, ++I) *I = A[i];
	}


	typedef T valType;
	constexpr static bool usesBuffer = true;

	[[nodiscard]] constexpr size_t size() const noexcept{ return len; }
	[[nodiscard]] constexpr size_t capacity() const noexcept{ return 0; }

	[[nodiscard]] constexpr valType operator [](const size_t i) const noexcept{ return *((T *)std::begin(MatrixTempStorage.data)+startIndex + i); }
};



template<class V1, class V2>
struct VectorExprAdd{
	static constexpr StupidVectorFlagType VectorFlag{};
	V1 arg1;
	V2 arg2;

	typedef std::remove_reference_t<V1> Arg1;
	typedef std::remove_reference_t<V2> Arg2;

	typedef typename Arg1::valType valType;
	constexpr static bool usesBuffer = Arg1::usesBuffer || Arg2::usesBuffer;

	[[nodiscard]] constexpr size_t size() const noexcept{ return arg1.size(); }
	[[nodiscard]] constexpr size_t capacity() const noexcept{ return 0; }

	[[nodiscard]] constexpr valType operator [](const size_t i) const noexcept{ return arg1[i] + arg2[i]; }
};
template<class V1, class V2>
struct VectorExprSubtract{
	static constexpr StupidVectorFlagType VectorFlag{};
	V1 arg1;
	V2 arg2;

	typedef std::remove_reference_t<V1> Arg1;
	typedef std::remove_reference_t<V2> Arg2;

	typedef typename Arg1::valType valType;
	constexpr static bool usesBuffer = Arg1::usesBuffer || Arg2::usesBuffer;

	[[nodiscard]] constexpr size_t size() const noexcept{ return arg1.size(); }
	[[nodiscard]] constexpr size_t capacity() const noexcept{ return 0; }

	[[nodiscard]] constexpr valType operator [](const size_t i) const noexcept{ return arg1[i] - arg2[i]; }
};
template<class V1, class V2>
struct VectorExprElMul{
	static constexpr StupidVectorFlagType VectorFlag{};
	V1 arg1;
	V2 arg2;

	typedef std::remove_reference_t<V1> Arg1;
	typedef std::remove_reference_t<V2> Arg2;

	typedef typename Arg1::valType valType;
	constexpr static bool usesBuffer = Arg1::usesBuffer || Arg2::usesBuffer;

	[[nodiscard]] constexpr size_t size() const noexcept{ return arg1.size(); }
	[[nodiscard]] constexpr size_t capacity() const noexcept{ return 0; }

	[[nodiscard]] constexpr valType operator [](const size_t i) const noexcept{ return arg1[i] * arg2[i]; }
};
template<class V1, class V2>
struct VectorExprElDiv{
	static constexpr StupidVectorFlagType VectorFlag{};
	V1 arg1;
	V2 arg2;

	typedef std::remove_reference_t<V1> Arg1;
	typedef std::remove_reference_t<V2> Arg2;

	typedef typename Arg1::valType valType;
	constexpr static bool usesBuffer = Arg1::usesBuffer || Arg2::usesBuffer;

	[[nodiscard]] constexpr size_t size() const noexcept{ return arg1.size(); }
	[[nodiscard]] constexpr size_t capacity() const noexcept{ return 0; }

	[[nodiscard]] constexpr valType operator [](const size_t i) const noexcept{ return arg1[i] / arg2[i]; }
};
template<class V1, class V2, auto operation>
struct VectorExprElStatOp{
	static constexpr StupidVectorFlagType VectorFlag{};
	V1 arg1;
	V2 arg2;

	typedef std::remove_reference_t<V1> Arg1;
	typedef std::remove_reference_t<V2> Arg2;

	typedef typename Arg1::valType valType;
	constexpr static bool usesBuffer = Arg1::usesBuffer || Arg2::usesBuffer;

	[[nodiscard]] constexpr size_t size() const noexcept{ return arg1.size(); }
	[[nodiscard]] constexpr size_t capacity() const noexcept{ return 0; }

	[[nodiscard]] constexpr valType operator [](const size_t i) const noexcept{ return operation(arg1[i], arg2[i]); }
};
template<class V1, class V2, class Operation>
struct VectorExprElDynOp{
	static constexpr StupidVectorFlagType VectorFlag{};
	V1 arg1;
	V2 arg2;
	Operation operation;


	typedef std::remove_reference_t<V1> Arg1;
	typedef std::remove_reference_t<V2> Arg2;

	typedef typename Arg1::valType valType;
	constexpr static bool usesBuffer = Arg1::usesBuffer || Arg2::usesBuffer;

	[[nodiscard]] constexpr size_t size() const noexcept{ return arg1.size(); }
	[[nodiscard]] constexpr size_t capacity() const noexcept{ return 0; }

	[[nodiscard]] constexpr valType operator [](const size_t i) const noexcept{ return operation(arg1[i], arg2[i]); }
};
template<class V>
struct VectorExprScalarMultiply{
	static constexpr StupidVectorFlagType VectorFlag{};
	typedef std::remove_reference_t<V> Arg;
	typedef typename Arg::valType valType;

	V arg1;
	valType arg2;

	constexpr static bool usesBuffer = Arg::usesBuffer;

	[[nodiscard]] constexpr size_t size() const noexcept{ return arg1.size(); }
	[[nodiscard]] constexpr size_t capacity() const noexcept{ return 0; }

	[[nodiscard]] constexpr valType operator [](const size_t i) const noexcept{ return arg1[i] * arg2; }
};


template<auto operation>
struct VectorExprStatGenerator{
	static constexpr StupidVectorFlagType VectorFlag{};
	uint32_t lenght;

	typedef decltype(operation(0)) valType;
	constexpr static bool usesBuffer = false;

	[[nodiscard]] constexpr size_t size() const noexcept{ return lenght; }
	[[nodiscard]] constexpr size_t capacity() const noexcept{ return 0; }

	[[nodiscard]] constexpr valType operator [](const size_t i) const noexcept{ return operation(i); }
};
template<class Operation>
struct VectorExprDynGenerator{
	static constexpr StupidVectorFlagType VectorFlag{};
	Operation operation;
	uint32_t lenght;

	typedef decltype(operation(0)) valType;
	constexpr static bool usesBuffer = false;

	[[nodiscard]] constexpr size_t size() const noexcept{ return lenght; }
	[[nodiscard]] constexpr size_t capacity() const noexcept{ return 0; }

	[[nodiscard]] constexpr valType operator [](const size_t i) const noexcept{ return operation(i); }
};
template<class T>
struct VectorExprUniformValue{
	static constexpr StupidVectorFlagType VectorFlag{};
	T value;
	uint32_t lenght;

	typedef T valType;
	constexpr static bool usesBuffer = false;

	[[nodiscard]] constexpr size_t size() const noexcept{ return lenght; }
	[[nodiscard]] constexpr size_t capacity() const noexcept{ return 0; }

	[[nodiscard]] constexpr valType operator [](const size_t i) const noexcept{ return value; }
};






} namespace sp{
using namespace priv__; 


template<SP_VECTOR_T(V), class Cont>
auto permute(V &&arg, const Cont &permuts) noexcept{
	SP_MATRIX_ERROR(size(arg) != std::size(permuts), "permutation array must have the same length as permuted vector");
	return VectorExprPermute<CRemRRef<V>, Cont, false>{arg, &permuts};
}
template<SP_VECTOR_T(V), class Cont>
auto lpermute(V &&arg, const Cont &permuts) noexcept{
	SP_MATRIX_ERROR(size(arg) != std::size(permuts), "permutation array must have the same length as permuted vector");
	return VectorExprPermute<RemRRef<V>, Cont, true>{arg, &permuts};
}

template<SP_VECTOR_T(V)>
auto cp(V &&arg) noexcept{
	return VectorExprCopy<typename std::decay_t<V>::valType>{arg};
}

template<auto operation, SP_VECTOR_T(V)>
auto apply(V &&arg) noexcept{
	return VectorExprElStatUnaryOp<CRemRRef<V>, operation>{arg};
}

template<SP_VECTOR_T(V), class Operation>
auto apply(V &&arg, Operation &&operation) noexcept{
	return VectorExprElDynUnaryOp<CRemRRef<V>, decltype(std::forward<Operation>(operation))>{arg, std::forward<Operation>(operation)};
}

template<SP_VECTOR_T(V1), SP_VECTOR_T(V2), class Operation>
auto apply(V1 &&lhs, V2 &&rhs, Operation &&operation) noexcept{
	SP_MATRIX_ERROR(size(lhs)!=size(rhs), "element wise operated vectors cannot have different dimensions");
	return VectorExprElDynOp<CRemRRef<V1>, CRemRRef<V2>, decltype(std::forward<Operation>(operation))>{lhs, rhs, std::forward<Operation>(operation)};
}


template<auto operation, SP_VECTOR_T(V1), SP_VECTOR_T(V2)>
auto apply(V1 &&lhs, V2 &&rhs) noexcept{
	SP_MATRIX_ERROR(size(lhs)!=size(rhs), "element wise operated vectors cannot have different dimensions");
	return VectorExprElStatOp<CRemRRef<V1>, CRemRRef<V2>, operation>{lhs, rhs};
}








template<auto operation>
[[nodiscard]] auto generate(const uint32_t size) noexcept{
	return VectorExprStatGenerator<operation>{size};
}
template<class Operation>
[[nodiscard]] auto generate(const uint32_t size, Operation &&operation) noexcept{
	return VectorExprDynGenerator<decltype(std::forward<Operation>(operation))>{std::forward<Operation>(operation), size};
}
template<class T>
[[nodiscard]] auto uniform(const uint32_t size, T value) noexcept{
	return VectorExprUniformValue<T>{value, size};
}







} namespace{
using namespace sp;
using namespace sp::priv__;

template<SP_VECTOR_T(V1), SP_VECTOR_T(V2)>
auto operator +(V1 &&lhs, V2 &&rhs) noexcept{
	SP_MATRIX_ERROR(size(lhs)!=size(rhs), "added vectors cannot have different dimensions");
	return VectorExprAdd<CRemRRef<V1>, CRemRRef<V2>>{lhs, rhs};
}

template<SP_VECTOR_T(V1), SP_VECTOR_T(V2)>
auto operator -(V1 &&lhs, V2 &&rhs) noexcept{
	SP_MATRIX_ERROR(size(lhs)!=size(rhs), "subtracted vectors cannot have different dimensions");
	return VectorExprSubtract<CRemRRef<V1>, CRemRRef<V2>>{lhs, rhs};
}


template<SP_VECTOR_T(V)>
auto operator *(V &&lhs, const typename std::decay_t<V>::valType &rhs) noexcept{
	return VectorExprScalarMultiply<CRemRRef<V>>{lhs, rhs};
}

template<SP_VECTOR_T(V)>
auto operator /(V &&lhs, const typename std::decay_t<V>::valType &rhs) noexcept{
	return VectorExprScalarMultiply<CRemRRef<V>>{lhs, (typename std::decay_t<V>::valType)1 / rhs};
}

} // END OF NAMESPACE ///////////////////////////////////////////////////////////////////