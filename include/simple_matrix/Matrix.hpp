#pragma once

#define SP_CI constexpr inline
#define SP_SI static inline
#define SP_CSI constexpr static inline


#include "Initializators.hpp"

#undef SP_CI
#undef SP_SI
#undef SP_CSI


template<class T>
using Matrix = MatrixWrapper<MatrixDynamic<T, MallocAllocator<>>>;

template<class T, size_t R, size_t C>
using FixedMatrix = MatrixWrapper<MatrixFixed<T, R, C>>;

template<class T, size_t C>
using FiniteMatrix = MatrixWrapper<MatrixFinite<T, C>>;

template<class T, class A = MallocAllocator<>>
using DynamicMatrix = MatrixWrapper<MatrixDynamic<T, A>>;




template<class T>
using Vector = VectorWrapper<VectorDynamic<T, MallocAllocator<>>>;

template<class T, size_t L>
using FixedVector = VectorWrapper<VectorFixed<T, L>>;

template<class T, size_t C>
using FiniteVector = VectorWrapper<VectorFinite<T, C>>;

template<class T, class A = MallocAllocator<>>
using DynamicVector = VectorWrapper<VectorDynamic<T, A>>;


