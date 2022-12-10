#pragma once
#include "MixedOperations.hpp"


template<SP_MATRIX_T(M), SP_VECTOR_T(V)>
void init_vandermonde(M &&dest, V &&A, const size_t degree) noexcept{
	typedef typename std::decay_t<M>::ValueType T;
	dest.resize(A.size(), degree+1);
	for (size_t i=0; i!=dest.rows(); ++i){
		dest(i, 0) = (T)1;
		T xVal = A[i];
		T accVal = xVal;
		for (size_t j=1; j!=dest.cols(); ++j){
			dest(i, j) = accVal;
			accVal *= xVal;
		}
	}
}

template<
	SP_MATRIX_T(M),
	class T = typename std::decay_t<M>::ValueType,
	class A = MallocAllocator<>
>
auto make_matrix(M &&m, A *allocator = nullptr){
	MatrixWrapper<MatrixDynamic<T, A>> res;
	res = m;
	return res;
}

template<class T = float, class A = MallocAllocator<>>
auto make_matrix(size_t r, size_t c, A *allocator = nullptr){
	MatrixWrapper<MatrixDynamic<T, A>> res;
	resize(res, r, c);
	return res;
}
