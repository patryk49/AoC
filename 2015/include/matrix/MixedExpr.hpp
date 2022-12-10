#pragma once

#include "Operations.hpp"


namespace sp::priv__{

template<class Base> struct Matrix;


template<class V, bool columnVector, bool isLVal>
struct MatrixExprAsMatrix{
	static constexpr StupidMatrixFlagType MatrixFlag{};
	V arg;

	typedef std::remove_reference_t<V> Arg;

	typedef typename Arg::valType valType;
	constexpr static bool isExpr = true;
	constexpr static bool rowMajor = columnVector;
	constexpr static bool undefMajor = false;
	constexpr static bool usesBuffer = Arg::usesBuffer;

	[[nodiscard]] constexpr size_t rows() const noexcept{ return columnVector ? arg.size() : 1; }
	[[nodiscard]] constexpr size_t cols() const noexcept{ return columnVector ? 1 : arg.size(); }
	[[nodiscard]] constexpr size_t size() const noexcept{ return arg.size(); }
	[[nodiscard]] constexpr size_t capacity() const noexcept{ return arg.capacity(); }

	constexpr void resize(const size_t r, const size_t c) noexcept{
		SP_MATRIX_ERROR(r!=1 || c!=1, "vector interpreted as matrix cannot be resized to non vector dimensions");
		arg.resize(r + c - 1);
	};

	constexpr std::conditional_t<isLVal, valType &, valType> operator ()(const size_t r, const size_t c) noexcept{ return arg[columnVector ? r : c]; }
	[[nodiscard]] constexpr valType operator ()(const size_t r, const size_t c) const noexcept{ return arg[columnVector ? r : c]; }
};

template<class V, bool isLVal>
struct MatrixExprAsDiagonalMatrix{
	static constexpr StupidMatrixFlagType MatrixFlag{};
	V arg;

	typedef std::remove_reference_t<V> Arg;

	typedef typename Arg::valType valType;
	constexpr static bool isExpr = true;
	constexpr static bool rowMajor = true;
	constexpr static bool undefMajor = true;
	constexpr static bool usesBuffer = Arg::usesBuffer;

	[[nodiscard]] constexpr size_t rows() const noexcept{ return arg.size(); }
	[[nodiscard]] constexpr size_t cols() const noexcept{ return arg.size(); }
	[[nodiscard]] constexpr size_t size() const noexcept{ return arg.size() * arg.size(); }
	[[nodiscard]] constexpr size_t capacity() const noexcept{ return arg.capacity(); }

	constexpr void resize(const size_t r, const size_t c) noexcept{
		SP_MATRIX_ERROR(r != c, "vector interpreted as diagonal matrix cannot be resized to non square dimensions");
		arg.resize(r);
	};

	constexpr std::enable_if_t<isLVal, valType &> operator ()(const size_t r, const size_t c) noexcept{ return arg[r]; }
	[[nodiscard]] constexpr valType operator ()(const size_t r, const size_t c) const noexcept{ return r==c ? arg[r] : (valType)0; }
};


template<class M, bool isLVal>
struct VectorExprAsRowVector{
	static constexpr StupidVectorFlagType VectorFlag{};
	M arg;
	size_t rowIndex;

	typedef std::remove_reference_t<M> Arg;

	typedef typename Arg::valType valType;
	constexpr static bool isExpr = Arg::isExpr;
	constexpr static bool usesBuffer = Arg::usesBuffer;

	[[nodiscard]] constexpr size_t size() const noexcept{ return arg.cols(); }
	[[nodiscard]] constexpr size_t capacity() const noexcept{ return arg.cols(); }

	constexpr void resize(const size_t n) noexcept{ SP_MATRIX_ERROR(n != arg.cols(), "vector made of matrix's row cannot be resized"); }

	constexpr std::conditional_t<isLVal, valType &, valType> operator [](const size_t i) noexcept{ return arg(rowIndex, i); }
	[[nodiscard]] constexpr valType operator [](const size_t i) const noexcept{ return arg(rowIndex, i); }
};

template<class M, bool isLVal>
struct VectorExprAsColumnVector{
	static constexpr StupidVectorFlagType VectorFlag{};
	M arg;
	size_t columnIndex;

	typedef std::remove_reference_t<M> Arg;

	typedef typename Arg::valType valType;
	constexpr static bool isExpr = Arg::isExpr;
	constexpr static bool usesBuffer = Arg::usesBuffer;

	[[nodiscard]] constexpr size_t size() const noexcept{ return arg.rows(); }
	[[nodiscard]] constexpr size_t capacity() const noexcept{ return arg.rows(); }

	constexpr void resize(const size_t n) noexcept{ SP_MATRIX_ERROR(n != arg.rows(), "vector made of matrix's column cannot be resized"); }

	constexpr std::conditional_t<isLVal, valType &, valType> operator [](const size_t i) noexcept{ return arg(i, columnIndex); }
	[[nodiscard]] constexpr valType operator [](const size_t i) const noexcept{ return arg(i, columnIndex); }
};


template<class M, class V>
struct VectorExprMatrixVertMultiply{
	static constexpr StupidVectorFlagType VectorFlag{};
	M arg1;
	V arg2;

	typedef std::remove_reference_t<M> Arg1;
	typedef std::remove_reference_t<V> Arg2;

	typedef typename Arg1::valType valType;
	constexpr static bool usesBuffer = Arg1::usesBuffer || Arg2::usesBuffer;

	[[nodiscard]] constexpr size_t size() const noexcept{ return arg1.rows(); }
	[[nodiscard]] constexpr size_t capacity() const noexcept{ return 0; }

	constexpr valType operator [](const size_t i) const noexcept{
		valType res = (valType)0;
		for (size_t j=0; j!=arg2.size(); ++j)
			res += arg1(i, j) * arg2[j];
		return res;
	}
};
template<class V, class M>
struct VectorExprMatrixHoriMultiply{
	static constexpr StupidVectorFlagType VectorFlag{};
	V arg1;
	M arg2;

	typedef std::remove_reference_t<V> Arg1;
	typedef std::remove_reference_t<M> Arg2;

	typedef typename Arg1::valType valType;
	constexpr static bool usesBuffer = Arg1::usesBuffer || Arg2::usesBuffer;

	[[nodiscard]] constexpr size_t size() const noexcept{ return arg1.cols(); }
	[[nodiscard]] constexpr size_t capacity() const noexcept{ return 0; }

	constexpr valType operator [](const size_t i) const noexcept{
		valType res = (valType)0;
		for (size_t j=0; j!=arg1.size(); ++j)
			res += arg1[j] * arg2(j, i);
		return res;
	}
};

template<class V1, class V2>
struct MatrixExprOuterProd{
	static constexpr StupidMatrixFlagType MatrixFlag{};
	V1 arg1;
	V2 arg2;

	typedef std::remove_reference_t<V1> Arg1;
	typedef std::remove_reference_t<V2> Arg2;

	typedef typename Arg1::valType valType;
	constexpr static bool rowMajor = false;
	constexpr static bool undefMajor = true;
	constexpr static bool usesBuffer = Arg1::usesBuffer || Arg2::usesBuffer;

	[[nodiscard]] constexpr size_t rows() const noexcept{ return arg1.size(); }
	[[nodiscard]] constexpr size_t cols() const noexcept{ return arg2.size(); }
	[[nodiscard]] constexpr size_t size() const noexcept{ return arg1.size() * arg2.size(); }
	[[nodiscard]] constexpr size_t capacity() const noexcept{ return 0; }

	constexpr valType operator ()(const size_t r, const size_t c) const noexcept{ return arg1[r] * arg2[c]; }
};
template<class V1, class V2, auto operation>
struct MatrixExprOuterStatOp{
	static constexpr StupidMatrixFlagType MatrixFlag{};
	V1 arg1;
	V2 arg2;

	typedef std::remove_reference_t<V1> Arg1;
	typedef std::remove_reference_t<V2> Arg2;

	typedef typename Arg1::valType valType;
	constexpr static bool rowMajor = false;
	constexpr static bool undefMajor = true;
	constexpr static bool usesBuffer = Arg1::usesBuffer || Arg2::usesBuffer;

	[[nodiscard]] constexpr size_t rows() const noexcept{ return arg1.size(); }
	[[nodiscard]] constexpr size_t cols() const noexcept{ return arg2.size(); }
	[[nodiscard]] constexpr size_t size() const noexcept{ return arg1.size() * arg2.size(); }
	[[nodiscard]] constexpr size_t capacity() const noexcept{ return 0; }

	constexpr valType operator ()(const size_t r, const size_t c) const noexcept{ return operation(arg1[r], arg2[c]); }
};
template<class V1, class V2, class Operation>
struct MatrixExprOuterDynOp{
	static constexpr StupidMatrixFlagType MatrixFlag{};
	V1 arg1;
	V2 arg2;
	Operation operation;

	typedef std::remove_reference_t<V1> Arg1;
	typedef std::remove_reference_t<V2> Arg2;

	typedef typename Arg1::valType valType;
	constexpr static bool rowMajor = false;
	constexpr static bool undefMajor = true;
	constexpr static bool usesBuffer = Arg1::usesBuffer || Arg2::usesBuffer;

	[[nodiscard]] constexpr size_t rows() const noexcept{ return arg1.size(); }
	[[nodiscard]] constexpr size_t cols() const noexcept{ return arg2.size(); }
	[[nodiscard]] constexpr size_t size() const noexcept{ return arg1.size() * arg2.size(); }
	[[nodiscard]] constexpr size_t capacity() const noexcept{ return 0; }

	constexpr valType operator ()(const size_t r, const size_t c) const noexcept{ return operation(arg1[r], arg2[c]); }
};

} namespace sp{
using namespace priv__;



template<SP_VECTOR_T(V)>
[[nodiscard]] auto asColumnMatrix(V &&arg) noexcept{
	return MatrixExprAsMatrix<CRemRRef<V>, true, false>{arg};
}
template<SP_VECTOR_T(V)>
auto lasColumnMatrix(V &&arg) noexcept{
	return Matrix<MatrixExprAsMatrix<RemRRef<V>, true, true>>{{arg}};
}

template<SP_VECTOR_T(V)>
[[nodiscard]] auto asRowMatrix(V &&arg) noexcept{
	return MatrixExprAsMatrix<CRemRRef<V>, false, false>{arg};
}
template<SP_VECTOR_T(V)>
auto lasRowMatrix(V &&arg) noexcept{
	return Matrix<MatrixExprAsMatrix<RemRRef<V>, false, true>>{{arg}};
}

template<SP_VECTOR_T(V)>
[[nodiscard]] auto asDiagonal(V &&arg) noexcept{
	return MatrixExprAsDiagonalMatrix<CRemRRef<V>, false>{arg};
}
template<SP_VECTOR_T(V)>
auto lasDiagonal(V &&arg) noexcept{
	return Matrix<MatrixExprAsDiagonalMatrix<RemRRef<V>, true>>{{arg}};
}

template<SP_MATRIX_T(M)>
[[nodiscard]] auto asRowVector(M &&arg, const size_t rowIndex) noexcept{
	SP_MATRIX_ERROR(rowIndex >= rows(arg), "row index exceeds the scope matrix as vector reinterpretation");
	return VectorExprAsRowVector<CRemRRef<M>, false>{arg, rowIndex};
}
template<SP_MATRIX_T(M)>
auto lasRowVector(M &&arg, const size_t rowIndex) noexcept{
	SP_MATRIX_ERROR(rowIndex >= rows(arg), "row index exceeds the scope matrix as vector reinterpretation");
	return VectorWrapper<VectorExprAsRowVector<RemRRef<M>, true>>{{arg, rowIndex}};
}

template<SP_MATRIX_T(M)>
[[nodiscard]] auto asColumnVector(M &&arg, const size_t columnIndex) noexcept{
	SP_MATRIX_ERROR(columnIndex >= rows(arg), "column index exceeds the scope matrix as vector reinterpretation");
	return VectorExprAsColumnVector<CRemRRef<M>, false>{arg, columnIndex};
}
template<SP_MATRIX_T(M)>
auto lasColumnVector(M &&arg, const size_t columnIndex) noexcept{
	SP_MATRIX_ERROR(columnIndex >= rows(arg), "column index exceeds the scope matrix as vector reinterpretation");
	return VectorWrapper<VectorExprAsColumnVector<RemRRef<M>, true>>{{arg, columnIndex}};
}


template<SP_VECTOR_T(V1), SP_VECTOR_T(V2)>
[[nodiscard]] auto outerProd(V1 &&lhs, V2 &&rhs) noexcept{
	return Matrix<MatrixExprOuterProd<CRemRRef<V1>, CRemRRef<V2>>>{{lhs, rhs}};
}

template<auto operation, SP_VECTOR_T(V1), SP_VECTOR_T(V2)>
[[nodiscard]] auto outerOp(V1 &&lhs, V2 &&rhs) noexcept{
	return Matrix<MatrixExprOuterStatOp<CRemRRef<V1>, CRemRRef<V2>, operation>>{{lhs, rhs}};
}

template<SP_VECTOR_T(V1), SP_VECTOR_T(V2), class Operation>
[[nodiscard]] auto outerOp(V1 &&lhs, V2 &&rhs, Operation &&operation) noexcept{
	return Matrix<MatrixExprOuterDynOp<CRemRRef<V1>, CRemRRef<V2>, decltype(std::forward<Operation>(operation))>>{{lhs, rhs, std::forward<Operation>(operation)}};
}


} namespace{
using namespace sp;
using namespace sp::priv__;

template<SP_MATRIX_T(M), SP_VECTOR_T(V)>
[[nodiscard]] auto operator *(M &&lhs, V &&rhs) noexcept{
	SP_MATRIX_ERROR(cols(lhs) != size(rhs), "vertically multipled vector must be of the same size as number of columns of multiplicating matrix");
	return VectorWrapper<VectorExprMatrixVertMultiply<CRemRRef<M>, CRemRRef<V>>>{{lhs, rhs}};
}

template<SP_VECTOR_T(V), SP_MATRIX_T(M)>
[[nodiscard]] auto operator *(V &&lhs, M &&rhs) noexcept{
	SP_MATRIX_ERROR(cols(lhs) != size(rhs), "horizontally multipled vector must be of the same size as number of columns of multiplicating matrix");
	return VectorWrapper<VectorExprMatrixHoriMultiply<CRemRRef<V>, CRemRRef<M>>>{{lhs, rhs}};
}



} // END OF NAMESPACE ///////////////////////////////////////////////////////////////////