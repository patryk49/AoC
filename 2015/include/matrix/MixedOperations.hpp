#pragma once

#include "MixedExpr.hpp"


namespace sp{



template<SP_VECTOR_T(V1), SP_MATRIX_T(M), class Cont, SP_VECTOR_T(V2)>
void lupSolve(V1 &&dest, M &&LU, const Cont permuts, V2 &&A) noexcept{
	SP_MATRIX_ERROR(rows(LU) != cols(LU), "only square matrix can be used as set of linear equations");
	SP_MATRIX_ERROR(rows(LU) != std::size(permuts), "permutaton array's size must be equal to number of rows of permuted matrix");
	const size_t len = rows(LU);

	dest.resize(len);
	for (size_t i=0; i!=len; ++i){
		dest[i] = A[(size_t)permuts[i]];

		for (size_t j=0; j!=i; ++j)
			dest[i] -= LU(i, j) * dest[j];
	}

	for (size_t i=len-1; i!=(size_t)-1; --i) {
		typename std::decay_t<M>::valType factor = (typename std::decay_t<M>::valType)1 / LU(i, i);
		for (size_t j = i+1; j!=len; ++j)
			dest[i] -= LU(i, j) * dest[j];

		dest[i] *= factor;
	}
}


template<SP_VECTOR_T(V), SP_MATRIX_T(M)>
void linSolve(V &&dest, M &&A) noexcept{
	SP_MATRIX_ERROR(rows(A) != cols(A), "only square matrix can hold a linear equations");
	const size_t len = rows(A);

	const size_t oldSize = std::size(priv__::MatrixTempStorage.data);
	priv__::MatrixTempStorage.data.expandBy((len*len * sizeof(typename std::decay_t<V>::valType) + 7) / 8);
	const size_t permutsIndex = std::size(priv__::MatrixTempStorage.data);
	priv__::MatrixTempStorage.data.expandBy((len * sizeof(uint32_t) + 7) / 8);
	typename std::decay_t<V>::valType *const TempStorage = (typename std::decay_t<V>::valType *)(std::begin(priv__::MatrixTempStorage.data) + oldSize);
	uint32_t *const permuts = (uint32_t *)(std::begin(priv__::MatrixTempStorage.data) + permutsIndex);

	for (size_t i=0; i!=len; ++i) permuts[i] = i;
	{
		typename std::decay_t<M>::valType *I = TempStorage;
		for (size_t i=0; i!=len; ++i)
			for (size_t j=0; j!=len; ++j, ++I)
				*I = A(i, j);
	}

	for (size_t i=0; i!=len; ++i){
		{
			size_t j = i;
			for (size_t k=i+1; k!=len; ++k)	// find row with max value
				j = abs(TempStorage[k*len+i])>abs(TempStorage[j*len+i]) ? k : j;
			if (j != i){
				for (size_t k=0; k!=len; ++k)	// exchange top row with row with max value
					std::swap(TempStorage[i*len+k], TempStorage[j*len+k]);
				std::swap(permuts[i], permuts[j]);
			}
		}
		typename std::decay_t<M>::valType factor1 = TempStorage[i*(len+1)];
		for (size_t j=i+1; j!=len; ++j){
			typename std::decay_t<M>::valType factor2 = TempStorage[j*len+i] / factor1;
			TempStorage[j*len+i] = factor2;
			for (size_t k=i+1; k!=len; ++k)
				TempStorage[j*len+k] -= TempStorage[i*len+k] * factor2;
		}
	}

	dest.resize(len, len);
	for (size_t i=0; i!=len; ++i){
		for (size_t j=0; j!=len; ++j){
			dest(j, i) = permuts[j]==i ? (typename std::decay_t<V>::valType)1 : (typename std::decay_t<V>::valType)0;

			for (size_t k=0; k!=j; ++k)
				dest(j, i) -= TempStorage[j*len+k] * dest(k, i);
		}

		for (size_t j=len-1; j!=(size_t)-1; --j) {
			typename std::decay_t<M>::valType factor = (typename std::decay_t<M>::valType)1 / TempStorage[j*(len+1)];
			for (size_t k = j+1; k!=len; ++k)
				dest(j, i) -= TempStorage[j*len+k] * dest(k, i);

			dest(j, i) *= factor;
		}
	}

	priv__::MatrixTempStorage.data.resize(oldSize);
}






template<SP_MATRIX_T(M), SP_VECTOR_T(V)>
[[nodiscard]] bool operator ==(M &&lhs, V &&rhs) noexcept{
	if (size(lhs) != size(rhs)) return false;
	if (rows(lhs) > cols(lhs)){
		if (cols(lhs) != 1) return false;
		for (size_t i=0; i!=size(rhs); ++i)
			if (lhs(i, 0) != rhs[i]) return false;
	} else{
		if (rows(lhs) != 1) return false;
		for (size_t i=0; i!=size(rhs); ++i)
			if (lhs(0, i) != rhs[i]) return false;
	}
	return true;
}

template<SP_MATRIX_T(M), SP_VECTOR_T(V)> [[nodiscard]] bool operator !=(M &&lhs, V &&rhs) noexcept{ return !(lhs == rhs); }
template<SP_VECTOR_T(V), SP_MATRIX_T(M)> [[nodiscard]] bool operator ==(V &&lhs, M &&rhs) noexcept{ return (rhs == lhs); }
template<SP_VECTOR_T(V), SP_MATRIX_T(M)> [[nodiscard]] bool operator !=(V &&lhs, M &&rhs) noexcept{ return !(rhs == lhs); }




} // END OF NAMESPACE ///////////////////////////////////////////////////////////////////