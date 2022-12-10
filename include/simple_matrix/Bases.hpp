#pragma once
#include <stdlib.h>

#include "SPL/Utils.hpp"
#include "SPL/Allocators.hpp"
#include "SPL/Arrays.hpp"

#ifdef SP_DEBUG
	#include <stdio.h>
	#define SP_MATRIX_ERROR(cond, msg) \
		if (cond){ fputs(msg, stderr); putc('\n', stderr); exit(1); }
#else
	#define SP_MATRIX_ERROR(cond, msg)
#endif


template<class T>
using RemRRef = std::remove_reference_t<T>;
template<class T>
using CRemRRef = std::remove_reference_t<std::add_const_t<T>>;


enum class StupidMatrixFlagType{};
enum class StupidVectorFlagType{};

#define SP_MATRIX_T(M) class M, StupidMatrixFlagType = std::decay_t<M>::MatrixFlag
#define SP_VECTOR_T(V) class V, StupidVectorFlagType = std::decay_t<V>::VectorFlag


struct DDAJSLdjsaldjaslkdjashdlDASLJD{
	DynamicArray<uint64_t, MallocAllocator<>{}> data = {{nullptr, 0}, 0};
	size_t stack_pos = 0;
} MatrixTempStorage;






template<class T, size_t rows, size_t cols>
struct MatrixFixed{
	typedef T ValueType;
	static constexpr StupidMatrixFlagType MatrixFlag{};
	constexpr static bool UsesBuffer = false;

	SP_CI T &operator ()(size_t r, size_t c) noexcept{
		SP_MATRIX_ERROR(r>=rows || c>=cols, "out of bounds matrix indecies");
		return data[r*cols + c];
	}

	SP_CI const T &operator ()(size_t r, size_t c) const noexcept{
		SP_MATRIX_ERROR(r>=rows || c>=cols, "out of bounds matrix indecies");
		return data[r*cols + c];
	}

	T data[rows * cols];
};

template<class T, size_t R, size_t C>
SP_CSI size_t rows(const MatrixFixed<T, R, C> &m) noexcept{ return R; }

template<class T, size_t R, size_t C>
SP_CSI size_t cols(const MatrixFixed<T, R, C> &m) noexcept{ return C; }

template<class T, size_t R, size_t C>
SP_CSI size_t len(const MatrixFixed<T, R, C> &m) noexcept{ return R * C; }

template<class T, size_t R, size_t C>
SP_CSI size_t cap(const MatrixFixed<T, R, C> &m) noexcept{ return 0; }

template<class T, size_t R, size_t C>
SP_CSI T *beg(const MatrixFixed<T, R, C> &m) noexcept{ return (T *)m.data; }

template<class T, size_t R, size_t C>
SP_CSI T *end(const MatrixFixed<T, R, C> &m) noexcept{ return (T *)m.data + R*C - 1; }

template<class T, size_t R, size_t C>
SP_SI void resize(
	MatrixFixed<T, R, C> &m, size_t r, size_t c
) noexcept{
	SP_MATRIX_ERROR(r!=R || c!=C, "static matrix cannot be resized");
}



template<class T, size_t cap>
struct MatrixFinite{
	typedef T ValueType;
	static constexpr StupidMatrixFlagType MatrixFlag{};
	constexpr static bool UsesBuffer = false;

	SP_CI T &operator ()(size_t r, size_t c) noexcept{
		SP_MATRIX_ERROR(r>=rows || c>=cols, "out of bounds matrix indecies");
		return data[r*(size_t)this->cols + c];
	}
	SP_CI const T &operator ()(size_t r, size_t c) const noexcept{
		SP_MATRIX_ERROR(r>=rows || c>=cols, "out of bounds matrix indecies");
		return data[r*(size_t)this->cols + c];
	}

	union{
		T data[cap];
	};
	uint32_t rows = 0;
	uint32_t cols = 0;
};

template<class T, size_t C>
SP_CSI size_t rows(const MatrixFinite<T, C> &m) noexcept{ return m.rows; }

template<class T, size_t C>
SP_CSI size_t cols(const MatrixFinite<T, C> &m) noexcept{ return m.cols; }

template<class T, size_t C>
SP_CSI size_t len(const MatrixFinite<T, C> &m) noexcept{ return m.rows * m.cols; }

template<class T, size_t C>
SP_CSI size_t cap(const MatrixFinite<T, C> &m) noexcept{ return C; }

template<class T, size_t C>
SP_CSI T *beg(const MatrixFinite<T, C> &m) noexcept{ return (T *)m.data; }

template<class T, size_t C>
SP_CSI T *end(const MatrixFinite<T, C> &m) noexcept{
	return (T *)m.data + m.rows*m.cols - 1;
}

template<class T, size_t C>
SP_SI void resize(MatrixFinite<T, C> &m, size_t r, size_t c) noexcept{
	SP_MATRIX_ERROR(r*c > C, "requested size exceeds the capacity");
	m.rows = r;
	m.cols = c;
}



template<class T, class A>
struct MatrixDynamic{
	typedef T ValueType;
	static constexpr StupidMatrixFlagType MatrixFlag{};
	constexpr static bool UsesBuffer = false;

	SP_CI T &operator ()(size_t r, size_t c) noexcept{
		SP_MATRIX_ERROR(r>=rows || c>=cols, "out of bounds matrix indecies");
		return *((T *)data.ptr + r*cols + c);
	}
	
	SP_CI const T &operator ()(size_t r, size_t c) const noexcept{
		SP_MATRIX_ERROR(r>=rows || c>=cols, "out of bounds matrix indecies");
		return *((const T *)data.ptr + r*cols + c);
	}

	Memblock data = {nullptr, 0};
	uint32_t rows = 0;
	uint32_t cols = 0;
	[[no_unique_address]] A *allocator = nullptr;
};

template<class T, class A>
SP_CSI size_t rows(const MatrixDynamic<T, A> &m) noexcept{ return m.rows; }

template<class T, class A>
SP_CSI size_t cols(const MatrixDynamic<T, A> &m) noexcept{ return m.cols; }

template<class T, class A>
SP_CSI size_t len(const MatrixDynamic<T, A> &m) noexcept{ return m.rows * m.cols; }

template<class T, class A>
SP_CSI size_t cap(const MatrixDynamic<T, A> &m) noexcept{ return m.data.size / sizeof(T); }

template<class T, class A>
SP_CSI T *beg(const MatrixDynamic<T, A> &m) noexcept{ return (T *)m.data.ptr; }

template<class T, class A>
SP_CSI T *end(const MatrixDynamic<T, A> &m) noexcept{
	return (T *)m.data.ptr + m.rows*m.cols - 1;
}

template<class T, class A>
SP_SI bool resize(
	MatrixDynamic<T, A> &m, size_t r, size_t c
) noexcept{
	size_t size = r * c * sizeof(T);
	if (m.data.size < size){
		Memblock blk;
		if constexpr (A::Alignment)
			blk = realloc(*m.allocator, m.data, size);
		else
			blk = realloc(*m.allocator, m.data, size, alignof(T));
		if (blk.ptr == nullptr) return true;
		m.data = blk;
	}
	
	m.rows = r;
	m.cols = c;
	return false;
}





template<class T, size_t size>
struct VectorFixed{
	typedef T ValueType;
	static constexpr StupidVectorFlagType VectorFlag{};
	constexpr static bool UsesBuffer = false;

	SP_CI T &operator [](size_t i) noexcept{
		SP_MATRIX_ERROR(i >= size, "out of bounds vector index");
		return data[i];
	}

	SP_CI const T &operator [](size_t i) const noexcept{
		SP_MATRIX_ERROR(i >= size, "out of bounds vector index");
		return data[i];
	}

	T data[size];
};

template<class T, size_t L>
SP_CSI size_t len(const VectorFixed<T, L> &v) noexcept{ return L; }

template<class T, size_t L>
SP_CSI size_t cap(const VectorFixed<T, L> &v) noexcept{ return 0; }

template<class T, size_t L>
SP_CSI T *beg(const VectorFixed<T, L> &v) noexcept{ return (T *)v.data; }

template<class T, size_t L>
SP_CSI T *end(const VectorFixed<T, L> &v) noexcept{ return (T *)v.data + L; }

template<class T, size_t L>
SP_SI void resize(VectorFixed<T, L> &v, size_t n) noexcept{
	SP_MATRIX_ERROR(n != L, "static vector cannot be resized");
}



template<class T, size_t cap>
struct VectorFinite{
	typedef T ValueType;
	static constexpr StupidVectorFlagType VectorFlag{};
	constexpr static bool UsesBuffer = false;

	SP_CI T &operator [](size_t i) noexcept{
		SP_MATRIX_ERROR(i >= size, "out of bounds vector index");
		return data[i];
	}

	SP_CI const T &operator [](size_t i) const noexcept{
		SP_MATRIX_ERROR(i >= size, "out of bounds vector index");
		return data[i];
	}


	union{
		T data[cap];
	};
	uint32_t size;
};

template<class T, size_t C>
SP_CSI size_t len(const VectorFinite<T, C> &v) noexcept{ return v.size; }

template<class T, size_t C>
SP_CSI size_t cap(const VectorFinite<T, C> &v) noexcept{ return C; }

template<class T, size_t C>
SP_CSI T *beg(const VectorFinite<T, C> &v) noexcept{ return (T *)v.data; }

template<class T, size_t C>
SP_CSI T *end(const VectorFinite<T, C> &v) noexcept{ return (T *)v.data + v.size; }

template<class T, size_t C>
SP_SI void resize(VectorFinite<T, C> &v, size_t n) noexcept{
	SP_MATRIX_ERROR(n>C, "requested size exceeds the capacity");
	v.size = n;
}



template<class T, class A>
struct VectorDynamic{
	typedef T ValueType;
	static constexpr StupidVectorFlagType VectorFlag{};
	constexpr static bool UsesBuffer = false;

	SP_CI T &operator [](size_t i) noexcept{
		SP_MATRIX_ERROR(i >= size, "out of bounds vector index");
		return *((T *)data.ptr + i);
	}
	SP_CI const T &operator [](size_t i) const noexcept{
		SP_MATRIX_ERROR(i >= size, "out of bounds vector index");
		return *((T *)data.ptr + i);
	}
	

	Memblock data = {nullptr, 0};
	size_t size = 0;
	A *allocator = nullptr;
};

template<class T, class A>
SP_CSI size_t len(const VectorDynamic<T, A> &v) noexcept{ return v.size; }

template<class T, class A>
SP_CSI size_t cap(const VectorDynamic<T, A> &v) noexcept{ return v.data.size / sizeof(T); }

template<class T, class A>
SP_CSI T *beg(const VectorDynamic<T, A> &v) noexcept{ return (T *)v.data.ptr; }

template<class T, class A>
SP_CSI T *end(const VectorDynamic<T, A> &v) noexcept{ return (T *)v.data.ptr + v.size; }

template<class T, class A>
SP_SI bool resize(VectorDynamic<T, A> &v, size_t l) noexcept{
	size_t size = l * sizeof(T);
	if (v.data.size < size){
		Memblock blk;
		if constexpr (A::Alignment)
			blk = realloc(*v.allocator, v.data, size);
		else
			blk = realloc(*v.allocator, v.data, size, alignof(T));
		if (blk.ptr == nullptr) return true;
		v.data = blk;
	}
	
	v.size = l;
	return false;
}



