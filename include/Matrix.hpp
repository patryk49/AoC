#pragma once

#include "SPL/Utils.hpp"
#include "SPL/Allocators.hpp"




Allocator matrix_allocator = (Allocator)MallocAllocator{};
Rand32 matrix_randomizer{};

enum class MatrixMajor{ Row, Col, Undef };

// MATRX BUFFER NOTATION:
// 1 << 0   - self
// 1 << 1   - arg
// 1 << 2   - right
// 1 << 3   - left


#ifdef SP_DEBUG
	#include <stdio.h>
	#define SP_MATRIX_ERROR(cond, msg) \
		if (cond){ \
			fputs("MATRIX_ERROR: ", stderr); \
			fputs(msg, stderr); \
			putc('\n', stderr); \
			exit(1); \
		}
#else
	#define SP_MATRIX_ERROR(cond, msg)
#endif

template<class Op, class... Args> constexpr
auto _impl_result_of_args_or_not(){
	if constexpr (std::is_invocable_v<Op, Args...>)
		return std::invoke_result_t<Op, Args...>{};
	else
		return std::invoke_result_t<Op>{};
}

template<class Op, class... Args>
using ResultWithArgsOrNot = decltype(_impl_result_of_args_or_not<Op, Args...>());


constexpr size_t CacheSize = 32768;
constexpr size_t CachePage = 64;

constexpr size_t CacheAvalible = CacheSize / 2;
constexpr size_t CacheBlockLen = int_sqrt(CacheAvalible);








template<class Base> struct MatrixExpr : Base{};

template<class Base>
struct MatrixBase : Base{
	template<class B>
	const MatrixBase &operator =(const MatrixBase<B> &rhs) noexcept{
		resize(*this, rows(rhs), cols(rhs));
		for (size_t i=0; i!=rows(*this); ++i)
			for (size_t j=0; j!=cols(*this); ++j)
				(*this)(i, j) = rhs(i, j);
		return *this;
	}
	template<class B>
	const MatrixBase &operator =(MatrixExpr<B> rhs) noexcept{
		resize(*this, rows(rhs), cols(rhs));
		for (size_t i=0; i!=rows(*this); ++i)
			for (size_t j=0; j!=cols(*this); ++j)
				(*this)(i, j) = rhs(i, j);
		free_matrix_expr((B)rhs);
		return *this;
	}

	template<class B>
	const MatrixBase &operator +=(const MatrixBase<B> &rhs) noexcept{
		for (size_t i=0; i!=rows(*this); ++i)
			for (size_t j=0; j!=cols(*this); ++j)
				(*this)(i, j) += rhs(i, j);
		return *this;
	}
	template<class B>
	const MatrixBase &operator +=(MatrixExpr<B> rhs) noexcept{
		for (size_t i=0; i!=rows(*this); ++i)
			for (size_t j=0; j!=cols(*this); ++j)
				(*this)(i, j) += rhs(i, j);
		free_matrix_expr((B)rhs);
		return *this;
	}

	template<class B>
	const MatrixBase &operator -=(const MatrixBase<B> &rhs) noexcept{
		for (size_t i=0; i!=rows(*this); ++i)
			for (size_t j=0; j!=cols(*this); ++j)
				(*this)(i, j) -= rhs(i, j);
		return *this;
	}
	template<class B>
	const MatrixBase &operator -=(MatrixExpr<B> rhs) noexcept{
		for (size_t i=0; i!=rows(*this); ++i)
			for (size_t j=0; j!=cols(*this); ++j)
				(*this)(i, j) -= rhs(i, j);
		free_matrix_expr((B)rhs);
		return *this;
	}

	template<class B>
	const MatrixBase &operator *=(const MatrixBase<B> &rhs) noexcept{
		size_t nrows = rows(*this);
		size_t ncols = cols(*this);
		resize(*this, nrows, cols(rhs));

		Memblock tmp = alloc(matrix_allocator, nrows*ncols*sizeof(typename Base::ValueType));
		for (size_t r=0; r!=nrows; r+=1)
			for (size_t c=0; c!=ncols; c+=1)
				*((typename Base::ValueType *)tmp.ptr+r*ncols+c) = (*this)(r, c);
		
		for (size_t c=0; c!=cols(*this); c+=1)
			for (size_t r=0; r!=rows(*this); r+=1){
				typename Base::ValueType sum = Null<typename Base::ValueType>;
				for (size_t k=0; k!=ncols; k+=1)
					sum += *((typename Base::ValueType *)tmp.ptr+r*ncols+k) * rhs(k, c);
				(*this)(r, c) = sum;
			}

		free(matrix_allocator, tmp);
		return *this;
	}
	template<class B>
	const MatrixBase &operator *=(MatrixExpr<B> rhs) noexcept{
		size_t nrows = rows(*this);
		size_t ncols = cols(*this);
		resize(*this, nrows, cols(rhs));

		Memblock tmp = alloc(matrix_allocator, nrows*ncols*sizeof(typename Base::ValueType));
		for (size_t r=0; r!=nrows; r+=1)
			for (size_t c=0; c!=ncols; c+=1)
				*((typename Base::ValueType *)tmp.ptr+r*ncols+c) = (*this)(r, c);
		
		for (size_t c=0; c!=cols(*this); c+=1)
			for (size_t r=0; r!=rows(*this); r+=1){
				typename Base::ValueType sum = Null<typename Base::ValueType>;
				for (size_t k=0; k!=ncols; k+=1)
					sum += *((typename Base::ValueType *)tmp.ptr+r*ncols+k) * rhs(k, c);
				(*this)(r, c) = sum;
			}

		free(matrix_allocator, tmp);
		free_matrix_expr((B)rhs);
		return *this;
	}
};


// RECURSIVE FREEING OF TEMPORARY ALLOCATIONS IN EXPRESSIONS
template<class M>
void free_matrix_expr(M expr) noexcept{
	if constexpr (M::Buffer != 0){
		if constexpr ((M::Buffer >> 0) & 1){
			free_matrix_expr_memblocks(expr);
		}
		if constexpr ((M::Buffer >> 1) & 1){
			free_matrix_expr(expr.arg);
		} else{
			if constexpr ((M::Buffer >> 3) & 1){
				free_matrix_expr(expr.lhs);
			}
			if constexpr ((M::Buffer >> 2) & 1){
				free_matrix_expr(expr.rhs);
			}
		}
	}
}









// MATRIX BASE CONTAINERS
template<class T, size_t R, size_t C = R, bool RowMajor = true>
struct MatrixFixed{
	typedef T ValueType;
	constexpr static uint32_t Buffer = 0;
	constexpr static MatrixMajor Major = RowMajor ? MatrixMajor::Row : MatrixMajor::Col;

	T &operator ()(size_t r, size_t c) noexcept{
		SP_MATRIX_ERROR(r>=R || c>=C, "out of bounds matrix indecies");
		if constexpr (RowMajor)
			return this->data[r*C + c];
		else
			return this->data[r + c*R];
	}

	const T &operator ()(size_t r, size_t c) const noexcept{
		SP_MATRIX_ERROR(r>=R || c>=C, "out of bounds matrix indecies");
		if constexpr (RowMajor)
			return this->data[r*C + c];
		else
			return this->data[r + c*R];
	}

	union{
		T data[R*C];
	};
};

template<class T, size_t R, size_t C, bool RM>
size_t rows(const MatrixFixed<T, R, C, RM> &) noexcept{ return R; }

template<class T, size_t R, size_t C, bool RM>
size_t cols(const MatrixFixed<T, R, C, RM> &) noexcept{ return C; }

template<class T, size_t R, size_t C, bool RM>
size_t len(const MatrixFixed<T, R, C, RM> &) noexcept{ return R*C; }

template<class T, size_t R, size_t C, bool RM>
size_t cap(const MatrixFixed<T, R, C, RM> &) noexcept{ return 0; }

template<class T, size_t R, size_t C, bool RM>
void resize(MatrixFixed<T, R, C, RM> &, size_t r, size_t c) noexcept{
	SP_MATRIX_ERROR(r!=R || c!=C, "static matrix cannot be resized");
}




template<class T, size_t C, bool RowMajor = true>
struct MatrixFinite{
	typedef T ValueType;
	constexpr static uint32_t Buffer = 0;
	constexpr static MatrixMajor Major = RowMajor ? MatrixMajor::Row : MatrixMajor::Col;

	T &operator ()(size_t r, size_t c) noexcept{
		SP_MATRIX_ERROR(r>=this->rows || c>=this->cols, "out of bounds matrix indecies");
		if constexpr (RowMajor)
			return this->data[r*this->cols + c];
		else
			return this->data[r + c*this->rows];
	}

	const T &operator ()(size_t r, size_t c) const noexcept{
		SP_MATRIX_ERROR(r>=this->rows || c>=this->cols, "out of bounds matrix indecies");
		if constexpr (RowMajor)
			return this->data[r*this->cols + c];
		else
			return this->data[r + c*this->rows];
	}

	union{
		T data[C];
	};
	uint32_t rows;
	uint32_t cols;
};

template<class T, size_t C, bool RM>
size_t rows(const MatrixFinite<T, C, RM> &m) noexcept{ return m.rows; }

template<class T, size_t C, bool RM>
size_t cols(const MatrixFinite<T, C, RM> &m) noexcept{ return m.cols; }

template<class T, size_t C, bool RM>
size_t len(const MatrixFinite<T, C, RM> &m) noexcept{ return m.rows * m.cols; }

template<class T, size_t C, bool RM>
size_t cap(const MatrixFinite<T, C, RM> &) noexcept{ return C; }

template<class T, size_t C, bool RM>
void resize(MatrixFinite<T, C, RM> &m, size_t r, size_t c) noexcept{
	SP_MATRIX_ERROR(r*c > C, "finite matrix does not have enough capacity");
	m.rows = r;
	m.cols = c;
}




template<class T, auto A = &matrix_allocator, bool RowMajor = true>
struct MatrixDynamic{
	typedef T ValueType;
	constexpr static uint32_t Buffer = 0;
	constexpr static MatrixMajor Major = RowMajor ? MatrixMajor::Row : MatrixMajor::Col;
	
	constexpr static auto Allocator = A;
	
	T &operator ()(size_t r, size_t c) noexcept{
		SP_MATRIX_ERROR(r>=this->rows || c>=this->cols, "out of bounds matrix indecies");
		if constexpr (RowMajor)
			return *((T *)this->data.ptr + r*this->cols + c);
		else
			return *((T *)this->data.ptr + r + c*this->rows);
	}

	const T &operator ()(size_t r, size_t c) const noexcept{
		SP_MATRIX_ERROR(r>=this->rows || c>=this->cols, "out of bounds matrix indecies");
		if constexpr (RowMajor)
			return *((const T *)this->data.ptr + r*this->cols + c);
		else
			return *((const T *)this->data.ptr + r + c*this->rows);
	}

	Memblock data = {nullptr, 0};
	uint32_t rows = 0;
	uint32_t cols = 0;
};

template<class T, auto A, bool RM>
size_t rows(const MatrixDynamic<T, A, RM> &m) noexcept{ return m.rows; }

template<class T, auto A, bool RM>
size_t cols(const MatrixDynamic<T, A, RM> &m) noexcept{ return m.cols; }

template<class T, auto A, bool RM>
size_t len(const MatrixDynamic<T, A, RM> &m) noexcept{ return m.rows * m.cols; }

template<class T, auto A, bool RM>
size_t cap(const MatrixDynamic<T, A, RM> &m) noexcept{ return m.data.size / sizeof(T); }

template<class T, auto A, bool RM>
bool resize(MatrixDynamic<T, A, RM> &m, size_t r, size_t c) noexcept{
	size_t size = r * c;
	if (m.rows*m.cols < size){
		size_t bytes = size * sizeof(T);
		if (m.data.size < bytes){
			Memblock blk;
			blk = realloc(deref(A), m.data, bytes);
			if (blk.ptr == nullptr) return true;
			m.data = blk;	
		}
	}
	m.rows = r;
	m.cols = c;
	return false;
}

template<class T, auto A, bool RM>
void free(MatrixDynamic<T, A, RM> &m) noexcept{
	free(deref(A), m.data);
	m.rows = 0;
	m.cols = 0;
}













// MATRIX EXPRESSION DATA STRUCTURES

template<class M>
struct MatrixTranspose{
	typedef std::decay_t<M> Arg;
	
	typedef typename Arg::ValueType ValueType;
	constexpr static uint32_t Buffer = (Arg::Buffer != 0) << 1;
	constexpr static MatrixMajor Major = (MatrixMajor [3]){
		MatrixMajor::Col, MatrixMajor::Row, MatrixMajor::Undef
	}[(size_t)Arg::Major];

	M arg;

	ValueType operator ()(size_t r, size_t c) const noexcept{ return this->arg(c, r); }
};

template<class M>
size_t rows(MatrixTranspose<M> m) noexcept{ return cols(m.arg); }

template<class M>
size_t cols(MatrixTranspose<M> m) noexcept{ return rows(m.arg); }

template<class M>
size_t len(MatrixTranspose<M> m) noexcept{ return len(m.arg); }




template<class M, class T>
struct MatrixPermuteRows{
	typedef std::decay_t<M> Arg;

	typedef typename Arg::ValueType ValueType;
	constexpr static uint32_t Buffer = (Arg::Buffer != 0) << 1;
	constexpr static MatrixMajor Major = Arg::Major;

	M arg;
	const T *permuts;

	ValueType operator ()(size_t r, size_t c) const noexcept{
		return arg(permuts[r], c);
	}
};

template<class M, class T>
size_t rows(MatrixPermuteRows<M, T> m) noexcept{ return rows(m.arg); }

template<class M, class T>
size_t cols(MatrixPermuteRows<M, T> m) noexcept{ return cols(m.arg); }

template<class M, class T>
size_t len(MatrixPermuteRows<M, T> m) noexcept{ return len(m.arg); }




template<class M, class T>
struct MatrixPermuteColumns{
	typedef std::decay_t<M> Arg;

	typedef typename Arg::ValueType ValueType;
	constexpr static uint32_t Buffer = (Arg::Buffer != 0) << 1;
	constexpr static MatrixMajor Major = Arg::Major;

	M arg;
	const T *permuts;

	ValueType operator ()(size_t r, size_t c) const noexcept{
		return arg(r, permuts[c]);
	}
};

template<class M, class T>
size_t rows(MatrixPermuteColumns<M, T> m) noexcept{ return rows(m.arg); }

template<class M, class T>
size_t cols(MatrixPermuteColumns<M, T> m) noexcept{ return cols(m.arg); }

template<class M, class T>
size_t len(MatrixPermuteColumns<M, T> m) noexcept{ return len(m.arg); }





template<class M, class T>
struct MatrixCast{
	typedef std::decay_t<M> Arg;
	
	typedef T ValueType;
	constexpr static uint32_t Buffer = (Arg::Buffer != 0) << 1;
	constexpr static MatrixMajor Major = Arg::Major;

	M arg;

	ValueType operator ()(size_t r, size_t c) const noexcept{ 
		return (T)this->arg(r, c);
	}
};

template<class M, class T>
size_t rows(MatrixCast<M, T> m) noexcept{ return rows(m.arg); }

template<class M, class T>
size_t cols(MatrixCast<M, T> m) noexcept{ return cols(m.arg); }

template<class M, class T>
size_t len(MatrixCast<M, T> m) noexcept{ return len(m.arg); }





template<class M, auto Operation>
struct MatrixUnaryStatOp{
	typedef std::decay_t<M> Arg;
	
	typedef typename Arg::ValueType ValueType;
	constexpr static uint32_t Buffer = (Arg::Buffer != 0) << 1;
	constexpr static MatrixMajor Major = Arg::Major;

	M arg;

	ValueType operator ()(size_t r, size_t c) const noexcept{ 
		if constexpr (std::is_invocable_v<decltype(Operation), ValueType>)
			return Operation(this->arg(r, c));
		else
			return Operation(this->arg(r, c), r, c);
	}
};

template<class M, auto Op>
size_t rows(MatrixUnaryStatOp<M, Op> m) noexcept{ return rows(m.arg); }

template<class M, auto Op>
size_t cols(MatrixUnaryStatOp<M, Op> m) noexcept{ return cols(m.arg); }

template<class M, auto Op>
size_t len(MatrixUnaryStatOp<M, Op> m) noexcept{ return len(m.arg); }



template<class M, class Operation>
struct MatrixUnaryDynOp{
	typedef std::decay_t<M> Arg;
	
	typedef typename Arg::ValueType ValueType;
	constexpr static uint32_t Buffer = (Arg::Buffer != 0) << 1;
	constexpr static MatrixMajor Major = Arg::Major;

	M arg;
	mutable Operation operation;
	
	ValueType operator ()(size_t r, size_t c) const noexcept{ 
		if constexpr (std::is_invocable_v<Operation, ValueType>)
			return this->operation(this->arg(r, c));
		else
			return this->operation(this->arg(r, c), r, c);
	}
};

template<class M, class Op>
size_t rows(MatrixUnaryDynOp<M, Op> m) noexcept{ return rows(m.arg); }

template<class M, class Op>
size_t cols(MatrixUnaryDynOp<M, Op> m) noexcept{ return cols(m.arg); }

template<class M, class Op>
size_t len(MatrixUnaryDynOp<M, Op> m) noexcept{ return len(m.arg); }



template<class T>
struct MatrixCopy{
	typedef T ValueType;
	constexpr static uint32_t Buffer = 1;
	constexpr static MatrixMajor Major = MatrixMajor::Row;

	Memblock memblock;
	uint32_t rows;
	uint32_t cols;

	T operator ()(size_t r, size_t c) const noexcept{
		SP_MATRIX_ERROR(r>=this->rows && c>=this->cols, "out of index matrix indexes");
		return *((T *)this->memblock.ptr + r*(size_t)this->cols + c);
	}
};

template<class T>
size_t rows(MatrixCopy<T> m) noexcept{ return m.rows; }

template<class T>
size_t cols(MatrixCopy<T> m) noexcept{ return m.cols; }

template<class T>
size_t len(MatrixCopy<T> m) noexcept{ return m.rows * m.cols; }


template<class T>
void free_matrix_expr_memblocks(MatrixCopy<T> m) noexcept{
	free(matrix_allocator, m.memblock);
}
//template<class T>
//void free_matrix_expr_memblocks(MatrixExpr<MatrixCopy<T>> m) noexcept{
//	free(matrix_allocator, m.memblock);
//}


template<class ML, class MR>
struct MatrixAdd{
	typedef std::decay_t<ML> Lhs;
	typedef std::decay_t<MR> Rhs;

	typedef typename Lhs::ValueType ValueType;
	constexpr static uint32_t Buffer = ((Lhs::Buffer != 0) << 3) | ((Rhs::Buffer != 0) << 2);
	constexpr static MatrixMajor Major = Lhs::Major!=MatrixMajor::Undef ? Lhs::Major : Rhs::Major;

	ML lhs;
	MR rhs;

	ValueType operator ()(size_t r, size_t c) const noexcept{
		return this->lhs(r, c) + this->rhs(r, c);
	}
};

template<class ML, class MR>
size_t rows(MatrixAdd<ML, MR> m) noexcept{ return rows(m.lhs); }

template<class ML, class MR>
size_t cols(MatrixAdd<ML, MR> m) noexcept{ return cols(m.lhs); }

template<class ML, class MR>
size_t len(MatrixAdd<ML, MR> m) noexcept{ return len(m.lhs); }




template<class ML, class MR>
struct MatrixSubtract{
	typedef std::decay_t<ML> Lhs;
	typedef std::decay_t<MR> Rhs;

	typedef typename Lhs::ValueType ValueType;
	constexpr static uint32_t Buffer = ((Lhs::Buffer != 0) << 3) | ((Rhs::Buffer != 0) << 2);
	constexpr static MatrixMajor Major = Lhs::Major!=MatrixMajor::Undef ? Lhs::Major : Rhs::Major;

	ML lhs;
	MR rhs;

	ValueType operator ()(size_t r, size_t c) const noexcept{
		return this->lhs(r, c) - this->rhs(r, c);
	}
};

template<class ML, class MR>
size_t rows(MatrixSubtract<ML, MR> m) noexcept{ return rows(m.lhs); }

template<class ML, class MR>
size_t cols(MatrixSubtract<ML, MR> m) noexcept{ return cols(m.lhs); }

template<class ML, class MR>
size_t len(MatrixSubtract<ML, MR> m) noexcept{ return len(m.lhs); }




template<class ML, class MR>
struct MatrixElMultiply{
	typedef std::decay_t<ML> Lhs;
	typedef std::decay_t<MR> Rhs;

	typedef typename Lhs::ValueType ValueType;
	constexpr static uint32_t Buffer = ((Lhs::Buffer != 0) << 3) | ((Rhs::Buffer != 0) << 2);
	constexpr static MatrixMajor Major = Lhs::Major!=MatrixMajor::Undef ? Lhs::Major : Rhs::Major;

	ML lhs;
	MR rhs;

	ValueType operator ()(size_t r, size_t c) const noexcept{
		return this->lhs(r, c) * this->rhs(r, c);
	}
};

template<class ML, class MR>
size_t rows(MatrixElMultiply<ML, MR> m) noexcept{ return rows(m.lhs); }

template<class ML, class MR>
size_t cols(MatrixElMultiply<ML, MR> m) noexcept{ return cols(m.lhs); }

template<class ML, class MR>
size_t len(MatrixElMultiply<ML, MR> m) noexcept{ return len(m.lhs); }




template<class ML, class MR>
struct MatrixElDivide{
	typedef std::decay_t<ML> Lhs;
	typedef std::decay_t<MR> Rhs;

	typedef typename Lhs::ValueType ValueType;
	constexpr static uint32_t Buffer = ((Lhs::Buffer != 0) << 3) | ((Rhs::Buffer != 0) << 2);
	constexpr static MatrixMajor Major = Lhs::Major!=MatrixMajor::Undef ? Lhs::Major : Rhs::Major;

	ML lhs;
	MR rhs;

	ValueType operator ()(size_t r, size_t c) const noexcept{
		return this->lhs(r, c) / this->rhs(r, c);
	}
};

template<class ML, class MR>
size_t rows(MatrixElDivide<ML, MR> m) noexcept{ return rows(m.lhs); }

template<class ML, class MR>
size_t cols(MatrixElDivide<ML, MR> m) noexcept{ return cols(m.lhs); }

template<class ML, class MR>
size_t len(MatrixElDivide<ML, MR> m) noexcept{ return len(m.lhs); }




template<class ML, class MR, auto Operation>
struct MatrixBinaryStatOp{
	typedef std::decay_t<ML> Lhs;
	typedef std::decay_t<MR> Rhs;

	typedef typename Lhs::ValueType ValueType;
	constexpr static uint32_t Buffer = ((Lhs::Buffer != 0) << 3) | ((Rhs::Buffer != 0) << 2);
	constexpr static MatrixMajor Major = Lhs::Major!=MatrixMajor::Undef ? Lhs::Major : Rhs::Major;

	ML lhs;
	MR rhs;

	ValueType operator ()(size_t r, size_t c) const noexcept{
		if constexpr (std::is_invocable_v<
				decltype(Operation), typename Lhs::ValueType, typename Rhs::ValueType
		>){
			return Operation(this->lhs(r, c), this->rhs(r, c));
		} else{
			return Operation(this->lhs(r, c), this->rhs(r, c), r, c);
		}
	}
};

template<class ML, class MR, auto Op>
size_t rows(MatrixBinaryStatOp<ML, MR, Op> m) noexcept{ return rows(m.lhs); }

template<class ML, class MR, auto Op>
size_t cols(MatrixBinaryStatOp<ML, MR, Op> m) noexcept{ return cols(m.lhs); }

template<class ML, class MR, auto Op>
size_t len(MatrixBinaryStatOp<ML, MR, Op> m) noexcept{ return len(m.lhs); }




template<class ML, class MR, class Operation>
struct MatrixBinaryDynOp{
	typedef std::decay_t<ML> Lhs;
	typedef std::decay_t<MR> Rhs;

	typedef typename Lhs::ValueType ValueType;
	constexpr static uint32_t Buffer = ((Lhs::Buffer != 0) << 3) | ((Rhs::Buffer != 0) << 2);
	constexpr static MatrixMajor Major = Lhs::Major!=MatrixMajor::Undef ? Lhs::Major : Rhs::Major;

	ML lhs;
	MR rhs;
	mutable Operation operation;

	ValueType operator ()(size_t r, size_t c) const noexcept{
		if constexpr (std::is_invocable_v<
			Operation, Lhs::ValueType, Rhs::ValueType
		>){
			return operation(this->lhs(r, c), this->rhs(r, c));
		} else{
			return operation(this->lhs(r, c), this->rhs(r, c), r, c);
		}
	}
};

template<class ML, class MR, class Op>
size_t rows(MatrixBinaryDynOp<ML, MR, Op> m) noexcept{ return rows(m.lhs); }

template<class ML, class MR, class Op>
size_t cols(MatrixBinaryDynOp<ML, MR, Op> m) noexcept{ return cols(m.lhs); }

template<class ML, class MR, class Op>
size_t len(MatrixBinaryDynOp<ML, MR, Op> m) noexcept{ return len(m.lhs); }




template<class ML, class MR>
struct MatrixMultiply{
	typedef std::decay_t<ML> Lhs;
	typedef std::decay_t<MR> Rhs;

	typedef typename Lhs::ValueType ValueType;
	constexpr static uint32_t Buffer = ((Lhs::Buffer != 0) << 3) | ((Rhs::Buffer != 0) << 2);
	constexpr static MatrixMajor Major = MatrixMajor::Undef;

	ML lhs;
	MR rhs;

	ValueType operator ()(size_t r, size_t c) const noexcept{
		ValueType res = Null<ValueType>;
		for (size_t i=0; i!=cols(lhs); ++i)
			res += lhs(r, i) * rhs(i, c);
		return res;
	}
};

template<class ML, class MR>
size_t rows(MatrixMultiply<ML, MR> m) noexcept{ return rows(m.lhs); }

template<class ML, class MR>
size_t cols(MatrixMultiply<ML, MR> m) noexcept{ return cols(m.rhs); }

template<class ML, class MR>
size_t len(MatrixMultiply<ML, MR> m) noexcept{ return rows(m.lhs) * cols(m.rhs); }




template<class M>
struct MatrixScalarMultiply{
	typedef std::decay_t<M> Arg;

	typedef typename Arg::ValueType ValueType;
	constexpr static uint32_t Buffer = (Arg::Buffer != 0) << 1;
	constexpr static MatrixMajor Major = Arg::Major;

	M arg;
	ValueType scalar;

	ValueType operator ()(size_t r, size_t c) const noexcept{
		return this->arg(r, c) * scalar;
	}
};

template<class M>
size_t rows(MatrixScalarMultiply<M> m) noexcept{ return rows(m.arg); }

template<class M>
size_t cols(MatrixScalarMultiply<M> m) noexcept{ return cols(m.arg); }

template<class M>
size_t len(MatrixScalarMultiply<M> m) noexcept{ return len(m.arg); }




template<class M>
struct MatrixScalarAdd{
	typedef std::decay_t<M> Arg;

	typedef typename Arg::ValueType ValueType;
	constexpr static uint32_t Buffer = (Arg::Buffer != 0) << 1;
	constexpr static MatrixMajor Major = Arg::Major;

	M arg;
	ValueType scalar;

	ValueType operator ()(size_t r, size_t c) const noexcept{
		return this->arg(r, c) + scalar;
	}
};

template<class M>
size_t rows(MatrixScalarAdd<M> m) noexcept{ return rows(m.arg); }

template<class M>
size_t cols(MatrixScalarAdd<M> m) noexcept{ return cols(m.arg); }

template<class M>
size_t len(MatrixScalarAdd<M> m) noexcept{ return len(m.arg); }




template<class M>
struct MatrixScalarSubtract{
	typedef std::decay_t<M> Arg;

	typedef typename Arg::ValueType ValueType;
	constexpr static uint32_t Buffer = (Arg::Buffer != 0) << 1;
	constexpr static MatrixMajor Major = Arg::Major;

	M arg;
	ValueType scalar;

	ValueType operator ()(size_t r, size_t c) const noexcept{
		return scalar - this->arg(r, c);
	}
};

template<class M>
size_t rows(MatrixScalarSubtract<M> m) noexcept{ return rows(m.arg); }

template<class M>
size_t cols(MatrixScalarSubtract<M> m) noexcept{ return cols(m.arg); }

template<class M>
size_t len(MatrixScalarSubtract<M> m) noexcept{ return len(m.arg); }




template<auto Operation>
struct MatrixStatGenerator{
	// typedef ValueType <LOOK LOWER>
	constexpr static uint32_t Buffer = 0;
	constexpr static MatrixMajor Major = MatrixMajor::Undef;

	uint32_t rows;
	uint32_t cols;
	
	auto operator ()(size_t r, size_t c) const noexcept{
		SP_MATRIX_ERROR(r>=this->rows || c>=this->cols, "out of bounds matrix indexes");
		if constexpr (std::is_invocable_v<decltype(Operation), size_t, size_t>)
			return Operation(r, c);
		else
			return Operation();
	}
	
	typedef ResultWithArgsOrNot<decltype(Operation), size_t, size_t> ValueType;
};

template<auto Op>
size_t rows(MatrixStatGenerator<Op> m) noexcept{ return m.rows; }

template<auto Op>
size_t cols(MatrixStatGenerator<Op> m) noexcept{ return m.cols; }

template<auto Op>
size_t len(MatrixStatGenerator<Op> m) noexcept{ return m.rows * m.cols; }




template<class Operation>
struct MatrixDynGenerator{
	// typedef ValueType <LOOK LOWER>
	constexpr static uint32_t Buffer = 0;
	constexpr static MatrixMajor Major = MatrixMajor::Undef;

	uint32_t rows;
	uint32_t cols;
	mutable Operation operation;
	
	auto operator ()(size_t r, size_t c) const noexcept{
		SP_MATRIX_ERROR(r>=this->rows || c>=this->cols, "out of bounds matrix indexes");
		if constexpr (std::is_invocable_v<Operation, size_t, size_t>)
			return operation(r, c);
		else
			return operation();
	}
	
	typedef ResultWithArgsOrNot<Operation, size_t, size_t> ValueType;
};

template<class Op>
size_t rows(MatrixDynGenerator<Op> m) noexcept{ return m.rows; }

template<class Op>
size_t cols(MatrixDynGenerator<Op> m) noexcept{ return m.cols; }

template<class Op>
size_t len(MatrixDynGenerator<Op> m) noexcept{ return m.rows * m.cols; }




template<class T>
struct MatrixUniformValue{
	typedef T ValueType;
	constexpr static uint32_t Buffer = 0;
	constexpr static MatrixMajor Major = MatrixMajor::Undef;

	uint32_t rows;
	uint32_t cols;
	T value;

	ValueType operator ()(size_t r, size_t c) const noexcept{
		SP_MATRIX_ERROR(r>=this->rows || c>=this->cols, "out of bounds matrix indexes");
		return value;
	}
};

template<class T>
size_t rows(MatrixUniformValue<T> m) noexcept{ return m.rows; }

template<class T>
size_t cols(const MatrixUniformValue<T> m) noexcept{ return m.cols; }

template<class T>
size_t len(const MatrixUniformValue<T> m) noexcept{ return m.rows * m.cols; }




template<class T>
struct MatrixRandomValue{
	typedef T ValueType;
	constexpr static uint32_t Buffer = 0;
	constexpr static MatrixMajor Major = MatrixMajor::Undef;

	uint32_t rows;
	uint32_t cols;
	T from;
	T span;

	ValueType operator ()(size_t r, size_t c) const noexcept{
		SP_MATRIX_ERROR(r>=this->rows || c>=this->cols, "out of bounds matrix indexes");
		if constexpr (std::is_floating_point_v<T>){
			return from + ((T)matrix_randomizer() / (T)max_val(matrix_randomizer)) * span;
		} else if constexpr (std::is_integral_v<T>){
			return from + (T)(matrix_randomizer() % (span + 1));
		}
	}
};

template<class T>
size_t rows(MatrixRandomValue<T> m) noexcept{ return m.rows; }

template<class T>
size_t cols(MatrixRandomValue<T> m) noexcept{ return m.cols; }

template<class T>
size_t len(MatrixRandomValue<T> m) noexcept{ return m.rows * m.cols; }






// MATRIX EXPRESSION OPERATIONS

template<class M>
auto tr(const MatrixBase<M> &arg) noexcept{
	return MatrixExpr<MatrixTranspose<const M &>>{arg};
}
template<class M>
auto tr(MatrixExpr<M> arg) noexcept{
	return MatrixExpr<MatrixTranspose<M>>{arg};
}




template<class M, class Cont>
auto perm_rows(const MatrixBase<M> &arg, const Cont &permuts) noexcept{
	SP_MATRIX_ERROR(
		rows(arg) != len(permuts),
		"permutation array must have the same length as number of permuted matrix's rows"
	);
	return MatrixExpr<MatrixPermuteRows<
		const M &, const decltype(permuts[0]) *
	>>{arg, beg(permuts)};
}
template<class M, class Cont>
auto perm_rows(MatrixExpr<M> arg, const Cont &permuts) noexcept{
	SP_MATRIX_ERROR(
		rows(arg) != len(permuts),
		"permutation array must have the same length as number of permuted matrix's rows"
	);
	return MatrixExpr<MatrixPermuteRows<
		M, const decltype(permuts[0]) *
	>>{arg, beg(permuts)};
}




template<class M, class Cont>
auto perm_cols(const MatrixBase<M> &arg, const Cont &permuts) noexcept{
	SP_MATRIX_ERROR(
		cols(arg) != len(permuts),
		"permutation array must have the same length as number of permuted matrix's columns"
	);
	return MatrixExpr<MatrixPermuteColumns<
		const M &, const decltype(permuts[0]) *
	>>{arg, beg(permuts)};
}
template<class M, class Cont>
auto perm_cols(MatrixExpr<M> arg, const Cont &permuts) noexcept{
	SP_MATRIX_ERROR(
		cols(arg) != len(permuts),
		"permutation array must have the same length as number of permuted matrix's columns"
	);
	return MatrixExpr<MatrixPermuteColumns<
		M, const decltype(permuts[0]) *
	>>{arg, beg(permuts)};
}




template<class T, class M>
auto cast(const MatrixBase<M> &arg) noexcept{
	return MatrixExpr<MatrixCast<const M &, T>>{arg};
}
template<class T, class M>
auto cast(MatrixExpr<M> arg) noexcept{
	return MatrixExpr<MatrixCast<M, T>>{arg};
}




// template<class M> > auto cp(const MatrixBase<M> &arg) <<<<<<<<  THIS MAKES NO SENSE  >>>>>>>>
template<class M>
auto cp(const MatrixBase<M> &arg) noexcept{
	using T = typename M::ValueType;
	MatrixCopy<T> res;
	res.rows = rows(arg);
	res.cols = cols(arg);
	res.memblock = alloc(matrix_allocator, res.rows*res.cols*sizeof(T));
	
	T *I = (T *)res.memblock.ptr;
	for (size_t i=0; i!=res.rows; ++i)
		for (size_t j=0; j!=res.cols; ++j, ++I)
			*I = arg(i, j);
	
	return MatrixExpr<MatrixCopy<T>>{res};
}
template<class M>
auto cp(MatrixExpr<M> arg) noexcept{
	using T = typename M::ValueType;
	MatrixCopy<T> res;
	res.rows = rows(arg);
	res.cols = cols(arg);
	res.memblock = alloc(matrix_allocator, res.rows*res.cols*sizeof(T));
	
	T *I = (T *)res.memblock.ptr;
	for (size_t i=0; i!=res.rows; ++i)
		for (size_t j=0; j!=res.cols; ++j, ++I)
			*I = arg(i, j);
	
	return MatrixExpr<MatrixCopy<T>>{res};
}




template<auto Op, class M>
auto apply(const MatrixBase<M> &arg) noexcept{
	return MatrixExpr<MatrixUnaryStatOp<const M &, Op>>{arg};
}
template<auto Op, class M>
auto apply(MatrixExpr<M> arg) noexcept{
	return MatrixExpr<MatrixUnaryStatOp<M, Op>>{arg};
}




template<class M, class Op>
auto apply(const MatrixBase<M> &arg, Op operation) noexcept{
	return MatrixExpr<MatrixUnaryDynOp<const M &, Op>>{arg, operation};
}
template<class M, class Op>
auto apply(MatrixExpr<M> arg, Op operation) noexcept{
	return MatrixExpr<MatrixUnaryDynOp<M, Op>>{arg, operation};
}




template<class ML, class MR>
auto elmul(const MatrixBase<ML> &lhs, const MatrixBase<MR> &rhs) noexcept{
	SP_MATRIX_ERROR(
		rows(lhs)!=rows(rhs) || cols(lhs)!=cols(rhs),
		"element wise multiplied matrices cannot have different dimensions"
	);
	return MatrixExpr<MatrixElMultiply<const ML &, const MR &>>{lhs, rhs};
}
template<class ML, class MR>
auto elmul(const MatrixBase<ML> &lhs, MatrixExpr<MR> rhs) noexcept{
	SP_MATRIX_ERROR(
		rows(lhs)!=rows(rhs) || cols(lhs)!=cols(rhs),
		"element wise multiplied matrices cannot have different dimensions"
	);
	return MatrixExpr<MatrixElMultiply<const ML &, MR>>{lhs, rhs};
}
template<class ML, class MR>
auto elmul(MatrixExpr<ML> lhs, const MatrixBase<MR> &rhs) noexcept{
	SP_MATRIX_ERROR(
		rows(lhs)!=rows(rhs) || cols(lhs)!=cols(rhs),
		"element wise multiplied matrices cannot have different dimensions"
	);
	return MatrixExpr<MatrixElMultiply<ML, const MR &>>{lhs, rhs};
}
template<class ML, class MR>
auto elmul(MatrixExpr<ML> lhs, MatrixExpr<MR> rhs) noexcept{
	SP_MATRIX_ERROR(
		rows(lhs)!=rows(rhs) || cols(lhs)!=cols(rhs),
		"element wise multiplied matrices cannot have different dimensions"
	);
	return MatrixExpr<MatrixElMultiply<ML, MR>>{lhs, rhs};
}




template<class ML, class MR>
auto eldiv(const MatrixBase<ML> &lhs, const MatrixBase<MR> &rhs) noexcept{
	SP_MATRIX_ERROR(
		rows(lhs)!=rows(rhs) || cols(lhs)!=cols(rhs),
		"element wise divided matrices cannot have different dimensions"
	);
	return MatrixExpr<MatrixElDivide<const ML &, const MR &>>{lhs, rhs};
}
template<class ML, class MR>
auto eldiv(const MatrixBase<ML> &lhs, MatrixExpr<MR> rhs) noexcept{
	SP_MATRIX_ERROR(
		rows(lhs)!=rows(rhs) || cols(lhs)!=cols(rhs),
		"element wise divided matrices cannot have different dimensions"
	);
	return MatrixExpr<MatrixElDivide<const ML &, MR>>{lhs, rhs};
}
template<class ML, class MR>
auto eldiv(MatrixExpr<ML> lhs, const MatrixBase<MR> &rhs) noexcept{
	SP_MATRIX_ERROR(
		rows(lhs)!=rows(rhs) || cols(lhs)!=cols(rhs),
		"element wise divided matrices cannot have different dimensions"
	);
	return MatrixExpr<MatrixElDivide<ML, const MR &>>{lhs, rhs};
}
template<class ML, class MR>
auto eldiv(MatrixExpr<ML> lhs, MatrixExpr<MR> rhs) noexcept{
	SP_MATRIX_ERROR(
		rows(lhs)!=rows(rhs) || cols(lhs)!=cols(rhs),
		"element wise divided matrices cannot have different dimensions"
	);
	return MatrixExpr<MatrixElDivide<ML, MR>>{lhs, rhs};
}




template<auto Op, class ML, class MR>
auto apply(const MatrixBase<ML> &lhs, const MatrixBase<MR> &rhs) noexcept{
	SP_MATRIX_ERROR(
		rows(lhs)!=rows(rhs) || cols(lhs)!=cols(rhs),
		"element wise operated matrices cannot have different dimensions"
	);
	return MatrixExpr<MatrixBinaryStatOp<const ML &, const MR &, Op>>{lhs, rhs};
}
template<auto Op, class ML, class MR>
auto apply(const MatrixBase<ML> &lhs, MatrixExpr<MR> rhs) noexcept{
	SP_MATRIX_ERROR(
		rows(lhs)!=rows(rhs) || cols(lhs)!=cols(rhs),
		"element wise operated matrices cannot have different dimensions"
	);
	return MatrixExpr<MatrixBinaryStatOp<const ML &, MR, Op>>{lhs, rhs};
}
template<auto Op, class ML, class MR>
auto apply(MatrixExpr<ML> lhs, const MatrixBase<MR> &rhs) noexcept{
	SP_MATRIX_ERROR(
		rows(lhs)!=rows(rhs) || cols(lhs)!=cols(rhs),
		"element wise operated matrices cannot have different dimensions"
	);
	return MatrixExpr<MatrixBinaryStatOp<ML, const MR &, Op>>{lhs, rhs};
}
template<auto Op, class ML, class MR>
auto apply(MatrixExpr<ML> lhs, MatrixExpr<MR> rhs) noexcept{
	SP_MATRIX_ERROR(
		rows(lhs)!=rows(rhs) || cols(lhs)!=cols(rhs),
		"element wise operated matrices cannot have different dimensions"
	);
	return MatrixExpr<MatrixBinaryStatOp<ML, MR, Op>>{lhs, rhs};
}




template<class ML, class MR, class Op>
auto apply(const MatrixBase<ML> &lhs, const MatrixBase<MR> &rhs, Op operation) noexcept{
	SP_MATRIX_ERROR(
		rows(lhs)!=rows(rhs) || cols(lhs)!=cols(rhs),
		"element wise operated matrices cannot have different dimensions"
	);
	return MatrixExpr<MatrixBinaryDynOp<const ML &, const MR &, Op>>{lhs, rhs, operation};
}
template<class ML, class MR, class Op>
auto apply(const MatrixBase<ML> &lhs, MatrixExpr<MR> rhs, Op operation) noexcept{
	SP_MATRIX_ERROR(
		rows(lhs)!=rows(rhs) || cols(lhs)!=cols(rhs),
		"element wise operated matrices cannot have different dimensions"
	);
	return MatrixExpr<MatrixBinaryDynOp<const ML &, MR, Op>>{lhs, rhs, operation};
}
template<class ML, class MR, class Op>
auto apply(MatrixExpr<ML> lhs, const MatrixBase<MR> &rhs, Op operation) noexcept{
	SP_MATRIX_ERROR(
		rows(lhs)!=rows(rhs) || cols(lhs)!=cols(rhs),
		"element wise operated matrices cannot have different dimensions"
	);
	return MatrixExpr<MatrixBinaryDynOp<ML, const MR &, Op>>{lhs, rhs, operation};
}
template<class ML, class MR, class Op>
auto apply(MatrixExpr<ML> lhs, MatrixExpr<MR> rhs, Op operation) noexcept{
	SP_MATRIX_ERROR(
		rows(lhs)!=rows(rhs) || cols(lhs)!=cols(rhs),
		"element wise operated matrices cannot have different dimensions"
	);
	return MatrixExpr<MatrixBinaryDynOp<ML, MR, Op>>{lhs, rhs, operation};
}




template<auto Op>
auto generate_matrix(size_t rows, size_t cols) noexcept{
	return MatrixExpr<MatrixStatGenerator<Op>>{rows, cols};
}




template<class Op>
auto generate_matrix(size_t rows, size_t cols, Op operation) noexcept{
	return MatrixExpr<MatrixDynGenerator<Op>>{rows, cols, operation};
}




template<class T = double>
auto uniform_matrix(size_t rows, size_t cols, T value = Null<T>) noexcept{
	return MatrixExpr<MatrixUniformValue<T>>{rows, cols, value};
}
template<class T = double>
auto uniform_matrix(size_t size, T value = Null<T>) noexcept{
	return MatrixExpr<MatrixUniformValue<T>>{size, size, value};
}




template<class T = double>
auto random_matrix(size_t rows, size_t cols, T from, T to) noexcept{
	return MatrixExpr<MatrixRandomValue<T>>{rows, cols, from, to-from};
}
template<class T = double>
auto random_matrix(size_t size, T from, T to) noexcept{
	return MatrixExpr<MatrixRandomValue<T>>{size, size, from, to-from};
}
template<class T = double>
auto random_matrix(size_t rows, size_t cols) noexcept{
	return MatrixExpr<MatrixRandomValue<T>>{rows, cols, Null<T>, Unit<T>};
}
template<class T = double>
auto random_matrix(size_t size) noexcept{
	return MatrixExpr<MatrixRandomValue<T>>{size, size, Null<T>, Unit<T>};
}




template<class ML, class MR>
auto operator +(const MatrixBase<ML> &lhs, const MatrixBase<MR> &rhs) noexcept{
	SP_MATRIX_ERROR(
		rows(lhs)!=rows(rhs) || cols(lhs)!=cols(rhs),
		"added matrices cannot have different dimensions"
	);
	return MatrixExpr<MatrixAdd<const ML &, const MR &>>{lhs, rhs};
}
template<class ML, class MR>
auto operator +(const MatrixBase<ML> &lhs, MatrixExpr<MR> rhs) noexcept{
	SP_MATRIX_ERROR(
		rows(lhs)!=rows(rhs) || cols(lhs)!=cols(rhs),
		"added matrices cannot have different dimensions"
	);
	return MatrixExpr<MatrixAdd<const ML &, MR>>{lhs, rhs};
}
template<class ML, class MR>
auto operator +(MatrixExpr<ML> lhs, const MatrixBase<MR> &rhs) noexcept{
	SP_MATRIX_ERROR(
		rows(lhs)!=rows(rhs) || cols(lhs)!=cols(rhs),
		"added matrices cannot have different dimensions"
	);
	return MatrixExpr<MatrixAdd<ML, const MR &>>{lhs, rhs};
}
template<class ML, class MR>
auto operator +(MatrixExpr<ML> lhs, MatrixExpr<MR> rhs) noexcept{
	SP_MATRIX_ERROR(
		rows(lhs)!=rows(rhs) || cols(lhs)!=cols(rhs),
		"added matrices cannot have different dimensions"
	);
	return MatrixExpr<MatrixAdd<ML, MR>>{lhs, rhs};
}




template<class ML, class MR>
auto operator -(const MatrixBase<ML> &lhs, const MatrixBase<MR> &rhs) noexcept{
	SP_MATRIX_ERROR(
		rows(lhs)!=rows(rhs) || cols(lhs)!=cols(rhs),
		"subtracted matrices cannot have different dimensions"
	);
	return MatrixExpr<MatrixSubtract<const ML &, const MR &>>{lhs, rhs};
}
template<class ML, class MR>
auto operator -(const MatrixBase<ML> &lhs, MatrixExpr<MR> rhs) noexcept{
	SP_MATRIX_ERROR(
		rows(lhs)!=rows(rhs) || cols(lhs)!=cols(rhs),
		"subtracted matrices cannot have different dimensions"
	);
	return MatrixExpr<MatrixSubtract<const ML &, MR>>{lhs, rhs};
}
template<class ML, class MR>
auto operator -(MatrixExpr<ML> lhs, const MatrixBase<MR> &rhs) noexcept{
	SP_MATRIX_ERROR(
		rows(lhs)!=rows(rhs) || cols(lhs)!=cols(rhs),
		"subtracted matrices cannot have different dimensions"
	);
	return MatrixExpr<MatrixSubtract<ML, const MR &>>{lhs, rhs};
}
template<class ML, class MR>
auto operator -(MatrixExpr<ML> lhs, MatrixExpr<MR> rhs) noexcept{
	SP_MATRIX_ERROR(
		rows(lhs)!=rows(rhs) || cols(lhs)!=cols(rhs),
		"subtracted matrices cannot have different dimensions"
	);
	return MatrixExpr<MatrixSubtract<ML, MR>>{lhs, rhs};
}




template<class ML, class MR>
auto operator *(const MatrixBase<ML> &lhs, const MatrixBase<MR> &rhs) noexcept{
	SP_MATRIX_ERROR(cols(lhs) != rows(rhs), "multiplied matrices have wrong dimensionss");
	return MatrixExpr<MatrixMultiply<const ML &, const MR &>>{lhs, rhs};
}
template<class ML, class MR>
auto operator *(const MatrixBase<ML> &lhs, MatrixExpr<MR> rhs) noexcept{
	SP_MATRIX_ERROR(cols(lhs) != rows(rhs), "multiplied matrices have wrong dimensionss");
	return MatrixExpr<MatrixMultiply<const ML &, MR>>{lhs, rhs};
}
template<class ML, class MR>
auto operator *(MatrixExpr<ML> lhs, const MatrixBase<MR> &rhs) noexcept{
	SP_MATRIX_ERROR(cols(lhs) != rows(rhs), "multiplied matrices have wrong dimensionss");
	return MatrixExpr<MatrixMultiply<ML, const MR &>>{lhs, rhs};
}
template<class ML, class MR>
auto operator *(MatrixExpr<ML> lhs, MatrixExpr<MR> rhs) noexcept{
	SP_MATRIX_ERROR(cols(lhs) != rows(rhs), "multiplied matrices have wrong dimensionss");
	return MatrixExpr<MatrixMultiply<ML, MR>>{lhs, rhs};
}




template<class M>
auto operator *(const MatrixBase<M> &arg, typename std::decay_t<M>::ValueType scalar) noexcept{
	return MatrixExpr<MatrixScalarMultiply<const M &>>{arg, scalar};
}
template<class M>
auto operator *(MatrixExpr<M> arg, typename std::decay_t<M>::ValueType scalar) noexcept{
	return MatrixExpr<MatrixScalarMultiply<M>>{arg, scalar};
}

template<class T, class M>
auto operator *(T scalar, const MatrixBase<M> &arg) noexcept{
	return MatrixExpr<MatrixScalarMultiply<const M &>>{
		arg, (typename std::decay_t<M>::ValueType)scalar
	};
}
template<class T, class M>
auto operator *(T scalar, MatrixExpr<M> arg) noexcept{
	return MatrixExpr<MatrixScalarMultiply<M>>{
		arg, (typename std::decay_t<M>::ValueType)scalar
	};
}




template<class M>
auto eladd(const MatrixBase<M> &arg, typename std::decay_t<M>::ValueType scalar) noexcept{
	return MatrixExpr<MatrixScalarAdd<const M &>>{arg, scalar};
}
template<class M>
auto eladd(MatrixExpr<M> arg, typename std::decay_t<M>::ValueType scalar) noexcept{
	return MatrixExpr<MatrixScalarAdd<M>>{arg, scalar};
}




template<class M>
auto elsub(const MatrixBase<M> &arg, typename std::decay_t<M>::ValueType scalar) noexcept{
	return MatrixExpr<MatrixScalarAdd<const M &>>{arg, -scalar};
}
template<class M>
auto elsub(MatrixExpr<M> arg, typename std::decay_t<M>::ValueType scalar) noexcept{
	return MatrixExpr<MatrixScalarAdd<M>>{arg, -scalar};
}

template<class T, class M>
auto elsub(T scalar, const MatrixBase<M> &arg) noexcept{
	return MatrixExpr<MatrixScalarSubtract<const M &>>{
		arg, (typename std::decay_t<M>::ValueType)scalar
	};
}
template<class T, class M>
auto elsub(T scalar, MatrixExpr<M> arg) noexcept{
	return MatrixExpr<MatrixScalarSubtract<M>>{
		arg, (typename std::decay_t<M>::ValueType)scalar
	};
}




template<class M>
auto operator /(const MatrixBase<M> &arg, typename std::decay_t<M>::ValueType scalar) noexcept{
	return MatrixExpr<MatrixScalarMultiply<const M &>>{arg, Unit<decltype(scalar)>/scalar};
}
template<class M>
auto operator /(MatrixExpr<M> arg, typename std::decay_t<M>::ValueType scalar) noexcept{
	return MatrixExpr<MatrixScalarMultiply<M>>{arg, Unit<decltype(scalar)>/scalar};
}












template<class Base> struct VectorExpr : Base{};

template<class Base>
struct VectorBase : Base{
	template<class B>
	const VectorBase &operator =(const VectorBase<B> &rhs) noexcept{
		resize(*this, len(rhs));
		for (size_t i=0; i!=len(*this); ++i)
			(*this)[i] = rhs[i];
		return *this;
	}
	template<class B>
	const VectorBase &operator =(VectorExpr<B> rhs) noexcept{
		resize(*this, len(rhs));
		for (size_t i=0; i!=len(*this); ++i)
			(*this)[i] = rhs[i];
		free_matrix_expr((B)rhs);
		return *this;
	}

	template<class B>
	const VectorBase &operator +=(const VectorBase<B> &rhs) noexcept{
		resize(*this, len(rhs));
		for (size_t i=0; i!=len(*this); ++i)
			(*this)[i] += rhs[i];
		return *this;
	}
	template<class B>
	const VectorBase &operator +=(VectorExpr<B> rhs) noexcept{
		resize(*this, len(rhs));
		for (size_t i=0; i!=len(*this); ++i)
			(*this)[i] += rhs[i];
		free_matrix_expr((B)rhs);
		return *this;
	}

	template<class B>
	const VectorBase &operator -=(const VectorBase<B> &rhs) noexcept{
		resize(*this, len(rhs));
		for (size_t i=0; i!=len(*this); ++i)
			(*this)[i] -= rhs[i];
		return *this;
	}
	template<class B>
	const VectorBase &operator -=(VectorExpr<B> rhs) noexcept{
		resize(*this, len(rhs));
		for (size_t i=0; i!=len(*this); ++i)
			(*this)[i] -= rhs[i];
		free_matrix_expr((B)rhs);
		return *this;
	}
};







// VECTOR BASE CONTAINERS
template<class T, size_t S>
struct VectorFixed{
	typedef T ValueType;
	constexpr static uint32_t Buffer = 0;

	T &operator [](size_t i) noexcept{
		SP_MATRIX_ERROR(i >= S, "out of bounds vector index");
		return this->data[i];
	}

	const T &operator [](size_t i) const noexcept{
		SP_MATRIX_ERROR(i >= S, "out of bounds vector index");
		return this->data[i];
	}

	union{
		T data[S];
	};
};

template<class T, size_t S>
size_t len(const VectorFixed<T, S> &) noexcept{ return S; }

template<class T, size_t S>
size_t cap(const VectorFixed<T, S> &) noexcept{ return 0; }

template<class T, size_t S>
void resize(VectorFixed<T, S> &, size_t size) noexcept{
	SP_MATRIX_ERROR(size != S, "static vector cannot be resized");
}




template<class T, size_t C>
struct VectorFinite{
	typedef T ValueType;
	constexpr static uint32_t Buffer = 0;

	T &operator [](size_t i) noexcept{
		SP_MATRIX_ERROR(i >= this->size, "out of bounds vector index");
		return this->data[i];
	}

	const T &operator [](size_t i) const noexcept{
		SP_MATRIX_ERROR(i >= this->size, "out of bounds vector index");
		return this->data[i];
	}

	union{
		T data[C];
	};
	uint32_t size;
};

template<class T, size_t C>
size_t len(const VectorFinite<T, C> &v) noexcept{ return v.size; }

template<class T, size_t C>
size_t cap(const VectorFinite<T, C> &) noexcept{ return C; }

template<class T, size_t C>
void resize(VectorFinite<T, C> &v, size_t size) noexcept{
	SP_MATRIX_ERROR(size > C, "finite vector does not have enough capacity");
	v.size = size;
}




template<class T, auto A = &matrix_allocator>
struct VectorDynamic{
	typedef T ValueType;
	constexpr static uint32_t Buffer = 0;
	
	constexpr static auto Allocator = A;
	
	T &operator [](size_t i) noexcept{
		SP_MATRIX_ERROR(i >= this->size, "out of bounds vector index");
		return *((T *)this->data.ptr + i);
	}

	const T &operator [](size_t i) const noexcept{
		SP_MATRIX_ERROR(i >= this->size, "out of bounds vector index");
		return *((const T *)this->data.ptr + i);
	}

	Memblock data = {nullptr, 0};
	size_t size;
};

template<class T, auto A>
size_t len(const VectorDynamic<T, A> &v) noexcept{ return v.size; }

template<class T, auto A>
size_t cap(const VectorDynamic<T, A> &v) noexcept{ return v.data.size / sizeof(T); }

template<class T, auto A>
bool resize(VectorDynamic<T, A> &v, size_t size) noexcept{
	if (v.size < size){
		size_t bytes = size * sizeof(T);
		if (v.data.size < bytes){
			Memblock blk;
			blk = realloc(deref(A), v.data, bytes);
			if (blk.ptr == nullptr) return true;
			v.data = blk;	
		}
	}
	v.size = size;
	return false;
}






// VECTOR EXPRESSION DATA STRUCTURES

template<class V, class T>
struct VectorPermute{
	typedef std::decay_t<V> Arg;

	typedef typename Arg::ValueType ValueType;
	constexpr static uint32_t Buffer = (Arg::Buffer != 0) << 1;

	V arg;
	const T *permuts;

	ValueType operator [](size_t i) const noexcept{
		return arg[permuts[i]];
	}
};

template<class V, class T>
size_t len(VectorPermute<V, T> v) noexcept{ return len(v.arg); }





template<class V, class T>
struct VectorCast{
	typedef std::decay_t<V> Arg;
	
	typedef T ValueType;
	constexpr static uint32_t Buffer = (Arg::Buffer != 0) << 1;

	V arg;

	ValueType operator [](size_t i) const noexcept{ 
		return (T)this->arg[i];
	}
};

template<class V, class T>
size_t len(VectorCast<V, T> v) noexcept{ return len(v.arg); }





template<class V, auto Operation>
struct VectorUnaryStatOp{
	typedef std::decay_t<V> Arg;
	
	typedef typename Arg::ValueType ValueType;
	constexpr static uint32_t Buffer = (Arg::Buffer != 0) << 1;

	V arg;

	ValueType operator [](size_t i) const noexcept{ 
		if constexpr (std::is_invocable_v<decltype(Operation), ValueType>)
			return Operation(this->arg[i]);
		else
			return Operation(this->arg[i], i);
	}
};

template<class V, auto Op>
size_t len(VectorUnaryStatOp<V, Op> v) noexcept{ return len(v.arg); }



template<class V, class Operation>
struct VectorUnaryDynOp{
	typedef std::decay_t<V> Arg;
	
	typedef typename Arg::ValueType ValueType;
	constexpr static uint32_t Buffer = (Arg::Buffer != 0) << 1;

	V arg;
	mutable Operation operation;

	ValueType operator [](size_t i) const noexcept{ 
		if constexpr (std::is_invocable_v<Operation, ValueType>)
			return this->operation(this->arg[i]);
		else
			return this->operation(this->arg[i], i);
	}
};

template<class V, class Op>
size_t len(VectorUnaryDynOp<V, Op> v) noexcept{ return len(v.arg); }



template<class T>
struct VectorCopy{
	typedef T ValueType;
	constexpr static uint32_t Buffer = 1;

	Memblock memblock;
	uint32_t size;

	T operator [](size_t i) const noexcept{
		SP_MATRIX_ERROR(i<this->size, "out of bound vector index");
		return *((T *)this->memblock.ptr + i);
	}
};

template<class T>
size_t len(VectorCopy<T> v) noexcept{ return v.rows * v.cols; }

template<class T>
void free_matrix_expr_memblocks(VectorCopy<T> m) noexcept{
	free(matrix_allocator, m.memblock);
}
//template<class T>
//void free_matrix_expr_memblocks(VectorExpr<VectorCopy<T>> v) noexcept{
//	free(matrix_allocator, v.memblock);
//}


template<class VL, class VR>
struct VectorAdd{
	typedef std::decay_t<VL> Lhs;
	typedef std::decay_t<VR> Rhs;

	typedef typename Lhs::ValueType ValueType;
	constexpr static uint32_t Buffer = ((Lhs::Buffer != 0) << 3) | ((Rhs::Buffer != 0) << 2);

	VL lhs;
	VR rhs;

	ValueType operator [](size_t i) const noexcept{
		return this->lhs[i] + this->rhs[i];
	}
};

template<class VL, class VR>
size_t len(VectorAdd<VL, VR> v) noexcept{ return len(v.lhs); }




template<class VL, class VR>
struct VectorSubtract{
	typedef std::decay_t<VL> Lhs;
	typedef std::decay_t<VR> Rhs;

	typedef typename Lhs::ValueType ValueType;
	constexpr static uint32_t Buffer = ((Lhs::Buffer != 0) << 3) | ((Rhs::Buffer != 0) << 2);

	VL lhs;
	VR rhs;

	ValueType operator [](size_t i) const noexcept{
		return this->lhs[i] - this->rhs[i];
	}
};

template<class VL, class VR>
size_t len(VectorExpr<VectorSubtract<VL, VR>> v) noexcept{ return len(v.lhs); }




template<class VL, class VR>
struct VectorElMultiply{
	typedef std::decay_t<VL> Lhs;
	typedef std::decay_t<VR> Rhs;

	typedef typename Lhs::ValueType ValueType;
	constexpr static uint32_t Buffer = ((Lhs::Buffer != 0) << 3) | ((Rhs::Buffer != 0) << 2);

	VL lhs;
	VR rhs;

	ValueType operator [](size_t i) const noexcept{
		return this->lhs[i] * this->rhs[i];
	}
};

template<class VL, class VR>
size_t len(VectorElMultiply<VL, VR> v) noexcept{ return len(v.lhs); }




template<class VL, class VR>
struct VectorElDivide{
	typedef std::decay_t<VL> Lhs;
	typedef std::decay_t<VR> Rhs;

	typedef typename Lhs::ValueType ValueType;
	constexpr static uint32_t Buffer = ((Lhs::Buffer != 0) << 3) | ((Rhs::Buffer != 0) << 2);

	VL lhs;
	VR rhs;

	ValueType operator [](size_t i) const noexcept{
		return this->lhs[i] / this->rhs[i];
	}
};

template<class VL, class VR>
size_t len(VectorElDivide<VL, VR> v) noexcept{ return len(v.lhs); }




template<class VL, class VR, auto Operation>
struct VectorBinaryStatOp{
	typedef std::decay_t<VL> Lhs;
	typedef std::decay_t<VR> Rhs;

	typedef typename Lhs::ValueType ValueType;
	constexpr static uint32_t Buffer = ((Lhs::Buffer != 0) << 3) | ((Rhs::Buffer != 0) << 2);

	VL lhs;
	VR rhs;

	ValueType operator [](size_t i) const noexcept{
		if constexpr (std::is_invocable_v<
				decltype(Operation), typename Lhs::ValueType, typename Rhs::ValueType
		>){
			return Operation(this->lhs[i], this->rhs[i]);
		} else{
			return Operation(this->lhs[i], this->rhs[i], i);
		}
	}
};

template<class VL, class VR, auto Op>
size_t len(VectorBinaryStatOp<VL, VR, Op> v) noexcept{ return len(v.lhs); }




template<class VL, class VR, class Operation>
struct VectorBinaryDynOp{
	typedef std::decay_t<VL> Lhs;
	typedef std::decay_t<VR> Rhs;

	typedef typename Lhs::ValueType ValueType;
	constexpr static uint32_t Buffer = ((Lhs::Buffer != 0) << 3) | ((Rhs::Buffer != 0) << 2);

	VL lhs;
	VR rhs;
	mutable Operation operation;

	ValueType operator [](size_t i) const noexcept{
		if constexpr (std::is_invocable_v<
			Operation, Lhs::ValueType, Rhs::ValueType
		>){
			return operation(this->lhs[i], this->rhs[i]);
		} else{
			return operation(this->lhs[i], this->rhs[i], i);
		}
	}
};

template<class VL, class VR, class Op>
size_t len(VectorBinaryDynOp<VL, VR, Op> v) noexcept{ return len(v.lhs); }






template<class V>
struct VectorScalarMultiply{
	typedef std::decay_t<V> Arg;

	typedef typename Arg::ValueType ValueType;
	constexpr static uint32_t Buffer = (Arg::Buffer != 0) << 1;

	V arg;
	ValueType scalar;

	ValueType operator [](size_t i) const noexcept{
		return this->arg[i] * scalar;
	}
};

template<class V>
size_t len(VectorScalarMultiply<V> v) noexcept{ return len(v.arg); }






template<class V>
struct VectorScalarAdd{
	typedef std::decay_t<V> Arg;

	typedef typename Arg::ValueType ValueType;
	constexpr static uint32_t Buffer = (Arg::Buffer != 0) << 1;

	V arg;
	ValueType scalar;

	ValueType operator [](size_t i) const noexcept{
		return this->arg[i] + scalar;
	}
};

template<class V>
size_t len(VectorScalarAdd<V> v) noexcept{ return len(v.arg); }






template<class V>
struct VectorScalarSubtract{
	typedef std::decay_t<V> Arg;

	typedef typename Arg::ValueType ValueType;
	constexpr static uint32_t Buffer = (Arg::Buffer != 0) << 1;

	V arg;
	ValueType scalar;

	ValueType operator [](size_t i) const noexcept{
		return scalar - this->arg[i];
	}
};

template<class V>
size_t len(VectorScalarSubtract<V> v) noexcept{ return len(v.arg); }






template<auto Operation>
struct VectorStatGenerator{
	// typedef ValueType <LOOK LOWER>
	constexpr static uint32_t Buffer = 0;

	uint32_t size;
	
	auto operator [](size_t i) const noexcept{
		SP_MATRIX_ERROR(i >= this->size, "out of bound vector index");
		if constexpr (std::is_invocable_v<decltype(Operation), size_t>)
			return Operation(i);
		else
			return Operation();
	}
	
	typedef ResultWithArgsOrNot<decltype(Operation), size_t> ValueType;
};

template<auto Op>
size_t len(VectorStatGenerator<Op> v) noexcept{ return v.size; }




template<class Operation>
struct VectorDynGenerator{
	// typedef ValueType <LOOK LOWER>
	constexpr static uint32_t Buffer = 0;

	uint32_t size;
	mutable Operation operation;
	
	auto operator [](size_t i) const noexcept{
		SP_MATRIX_ERROR(i >= this->size, "out of bound vector index");
		if constexpr (std::is_invocable_v<Operation, size_t>)
			return operation(i);
		else
			return operation();
	}
	
	typedef ResultWithArgsOrNot<Operation, size_t> ValueType;
};

template<class Op>
size_t len(VectorDynGenerator<Op> v) noexcept{ return v.size; }




template<class T>
struct VectorUniformValue{
	typedef T ValueType;
	constexpr static uint32_t Buffer = 0;

	uint32_t size;
	T value;

	ValueType operator [](size_t i) const noexcept{
		SP_MATRIX_ERROR(i >= this->size, "out of bounds vector index");
		return value;
	}
};

template<class T>
size_t len(const VectorUniformValue<T> v) noexcept{ return v.size; }




template<class T>
struct VectorRandomValue{
	typedef T ValueType;
	constexpr static uint32_t Buffer = 0;

	uint32_t size;
	T from;
	T span;

	ValueType operator [](size_t i) const noexcept{
		SP_MATRIX_ERROR(i >= this->size, "out of bound vector index");
		if constexpr (std::is_floating_point_v<T>){
			return from + ((T)matrix_randomizer() / (T)max_val(matrix_randomizer)) * span;
		} else if constexpr (std::is_integral_v<T>){
			return from + (T)(matrix_randomizer() % (span + 1));
		}
	}
};

template<class T>
size_t len(const VectorRandomValue<T> v) noexcept{ return v.size; }






// VECTOR EXPRESSION OPERATIONS

template<class V, class Cont>
auto perm(const VectorBase<V> &arg, const Cont &permuts) noexcept{
	SP_MATRIX_ERROR(
		len(arg) != len(permuts),
		"permutation array must have the same length as number of permuted vector"
	);
	return VectorExpr<VectorPermute<
		const V &, const decltype(permuts[0]) *
	>>{arg, beg(permuts)};
}
template<class V, class Cont>
auto perm(VectorExpr<V> arg, const Cont &permuts) noexcept{
	SP_MATRIX_ERROR(
		len(arg) != len(permuts),
		"permutation array must have the same length as number of permuted vector"
	);
	return VectorExpr<VectorPermute<
		V, const decltype(permuts[0]) *
	>>{arg, beg(permuts)};
}




template<class T, class V>
auto cast(const VectorBase<V> &arg) noexcept{
	return VectorExpr<VectorCast<const V &, T>>{arg};
}
template<class T, class V>
auto cast(VectorExpr<V> arg) noexcept{
	return VectorExpr<VectorCast<V, T>>{arg};
}




// template<class V> > auto cp(const VectorBase<V> &arg) <<<<<<<<  THIS MAKES NO SENSE  >>>>>>>>
template<class V>
auto cp(VectorExpr<V> arg) noexcept{
	using T = typename V::ValueType;
	VectorCopy<T> res;
	res.size = len(arg);
	res.memblock = alloc(matrix_allocator, res.size*sizeof(T));
	
	T *I = (T *)res.memblock.ptr;
	for (size_t i=0; i!=res.size; ++i, ++I)
		*I = arg[i];
	
	return VectorExpr<VectorCopy<T>>{res};
}
template<class T, class V>  // copying can be used for type casting
auto cp(VectorExpr<V> arg) noexcept{
	VectorCopy<T> res;
	res.size = len(arg);
	res.memblock = alloc(matrix_allocator, res.size*sizeof(T));

	T *I = (T *)res.memblock.ptr;
	for (size_t i=0; i!=res.size; ++i)
		*I = (T)arg[i];
	
	return VectorExpr<VectorCopy<T>>{res};
}




template<auto Op, class V>
auto apply(const VectorBase<V> &arg) noexcept{
	return VectorExpr<VectorUnaryStatOp<const V &, Op>>{arg};
}
template<auto Op, class V>
auto apply(VectorExpr<V> arg) noexcept{
	return VectorExpr<VectorUnaryStatOp<V, Op>>{arg};
}




template<class V, class Op>
auto apply(const VectorBase<V> &arg, Op operation) noexcept{
	return VectorExpr<VectorUnaryDynOp<const V &, Op>>{arg, operation};
}
template<class V, class Op>
auto apply(VectorExpr<V> arg, Op operation) noexcept{
	return VectorExpr<VectorUnaryDynOp<V, Op>>{arg, operation};
}




template<class VL, class VR>
auto elmul(const VectorBase<VL> &lhs, const VectorBase<VR> &rhs) noexcept{
	SP_MATRIX_ERROR(
		len(lhs) != len(rhs),
		"element wise multiplied vectors cannot have different dimensions"
	);
	return VectorExpr<VectorElMultiply<const VL &, const VR &>>{lhs, rhs};
}
template<class VL, class VR>
auto elmul(const VectorBase<VL> &lhs, VectorExpr<VR> rhs) noexcept{
	SP_MATRIX_ERROR(
		len(lhs) != len(rhs),
		"element wise multiplied vectors cannot have different dimensions"
	);
	return VectorExpr<VectorElMultiply<const VL &, VR>>{lhs, rhs};
}
template<class VL, class VR>
auto elmul(VectorExpr<VL> lhs, const VectorBase<VR> &rhs) noexcept{
	SP_MATRIX_ERROR(
		len(lhs) != len(rhs),
		"element wise multiplied vectors cannot have different dimensions"
	);
	return VectorExpr<VectorElMultiply<VL, const VR &>>{lhs, rhs};
}
template<class VL, class VR>
auto elmul(VectorExpr<VL> lhs, VectorExpr<VR> rhs) noexcept{
	SP_MATRIX_ERROR(
		len(lhs) != len(rhs),
		"element wise multiplied vectors cannot have different dimensions"
	);
	return VectorExpr<VectorElMultiply<VL, VR>>{lhs, rhs};
}




template<class VL, class VR>
auto eldiv(const VectorBase<VL> &lhs, const VectorBase<VR> &rhs) noexcept{
	SP_MATRIX_ERROR(
		len(lhs) != len(rhs),
		"element wise divided vectors cannot have different dimensions"
	);
	return VectorExpr<VectorElDivide<const VL &, const VR &>>{lhs, rhs};
}
template<class VL, class VR>
auto eldiv(const VectorBase<VL> &lhs, VectorExpr<VR> rhs) noexcept{
	SP_MATRIX_ERROR(
		len(lhs) != len(rhs),
		"element wise divided vectors cannot have different dimensions"
	);
	return VectorExpr<VectorElDivide<const VL &, VR>>{lhs, rhs};
}
template<class VL, class VR>
auto eldiv(VectorExpr<VL> lhs, const VectorBase<VR> &rhs) noexcept{
	SP_MATRIX_ERROR(
		len(lhs) != len(rhs),
		"element wise divided vectors cannot have different dimensions"
	);
	return VectorExpr<VectorElDivide<VL, const VR &>>{lhs, rhs};
}
template<class VL, class VR>
auto eldiv(VectorExpr<VL> lhs, VectorExpr<VR> rhs) noexcept{
	SP_MATRIX_ERROR(
		len(lhs) != len(rhs),
		"element wise divided vectors cannot have different dimensions"
	);
	return VectorExpr<VectorElDivide<VL, VR>>{lhs, rhs};
}




template<auto Op, class VL, class VR>
auto apply(const VectorBase<VL> &lhs, const VectorBase<VR> &rhs) noexcept{
	SP_MATRIX_ERROR(
		len(lhs) != len(rhs),
		"element wise operated vectors cannot have different dimensions"
	);
	return VectorExpr<VectorBinaryStatOp<const VL &, const VR &, Op>>{lhs, rhs};
}
template<auto Op, class VL, class VR>
auto apply(const VectorBase<VL> &lhs, VectorExpr<VR> rhs) noexcept{
	SP_MATRIX_ERROR(
		len(lhs) != len(rhs),
		"element wise operated vectors cannot have different dimensions"
	);
	return VectorExpr<VectorBinaryStatOp<const VL &, VR, Op>>{lhs, rhs};
}
template<auto Op, class VL, class VR>
auto apply(VectorExpr<VL> lhs, const VectorBase<VR> &rhs) noexcept{
	SP_MATRIX_ERROR(
		len(lhs) != len(rhs),
		"element wise operated vectors cannot have different dimensions"
	);
	return VectorExpr<VectorBinaryStatOp<VL, const VR &, Op>>{lhs, rhs};
}
template<auto Op, class VL, class VR>
auto apply(VectorExpr<VL> lhs, VectorExpr<VR> rhs) noexcept{
	SP_MATRIX_ERROR(
		len(lhs) != len(rhs),
		"element wise operated vectors cannot have different dimensions"
	);
	return VectorExpr<VectorBinaryStatOp<VL, VR, Op>>{lhs, rhs};
}




template<class VL, class VR, class Op>
auto apply(const VectorBase<VL> &lhs, const VectorBase<VR> &rhs, Op operation) noexcept{
	SP_MATRIX_ERROR(
		rows(lhs)!=rows(rhs) || cols(lhs)!=cols(rhs),
		"element wise operated vectorss cannot have different dimensions"
	);
	return VectorExpr<VectorBinaryDynOp<const VL &, const VR &, Op>>{lhs, rhs, operation};
}
template<class VL, class VR, class Op>
auto apply(const VectorBase<VL> &lhs, VectorExpr<VR> rhs, Op operation) noexcept{
	SP_MATRIX_ERROR(
		rows(lhs)!=rows(rhs) || cols(lhs)!=cols(rhs),
		"element wise operated vectors cannot have different dimensions"
	);
	return VectorExpr<VectorBinaryDynOp<const VL &, VR, Op>>{lhs, rhs, operation};
}
template<class VL, class VR, class Op>
auto apply(VectorExpr<VL> lhs, const VectorBase<VR> &rhs, Op operation) noexcept{
	SP_MATRIX_ERROR(
		rows(lhs)!=rows(rhs) || cols(lhs)!=cols(rhs),
		"element wise operated vectors cannot have different dimensions"
	);
	return VectorExpr<VectorBinaryDynOp<VL, const VR &, Op>>{lhs, rhs, operation};
}
template<class VL, class VR, class Op>
auto apply(VectorExpr<VL> lhs, VectorExpr<VR> rhs, Op operation) noexcept{
	SP_MATRIX_ERROR(
		rows(lhs)!=rows(rhs) || cols(lhs)!=cols(rhs),
		"element wise operated vectors cannot have different dimensions"
	);
	return VectorExpr<VectorBinaryDynOp<VL, VR, Op>>{lhs, rhs, operation};
}




template<auto Op>
auto generate(size_t size) noexcept{
	return VectorExpr<VectorStatGenerator<Op>>{size};
}




template<class Op>
auto generate_vector(size_t size, Op operation) noexcept{
	return VectorExpr<VectorDynGenerator<Op>>{size, operation};
}




template<class T = double>
auto uniform_vector(size_t size, T value = Null<T>) noexcept{
	return VectorExpr<VectorUniformValue<T>>{size, value};
}




template<class T = double>
auto random_vector(size_t size, T from, T to) noexcept{
	return VectorExpr<VectorRandomValue<T>>{size, from, to-from};
}
template<class T = double>
auto random_vector(size_t size) noexcept{
	return VectorExpr<VectorRandomValue<T>>{size, Null<T>, Unit<T>};
}




template<class VL, class VR>
auto operator +(const VectorBase<VL> &lhs, const VectorBase<VR> &rhs) noexcept{
	SP_MATRIX_ERROR(
		len(lhs) != len(rhs),
		"added vectors cannot have different dimensions"
	);
	return VectorExpr<VectorAdd<const VL &, const VR &>>{lhs, rhs};
}
template<class VL, class VR>
auto operator +(const VectorBase<VL> &lhs, VectorExpr<VR> rhs) noexcept{
	SP_MATRIX_ERROR(
		len(lhs) != len(rhs),
		"added vectors cannot have different dimensions"
	);
	return VectorExpr<VectorAdd<const VL &, VR>>{lhs, rhs};
}
template<class VL, class VR>
auto operator +(VectorExpr<VL> lhs, const VectorBase<VR> &rhs) noexcept{
	SP_MATRIX_ERROR(
		len(lhs) != len(rhs),
		"added vectors cannot have different dimensions"
	);
	return VectorExpr<VectorAdd<VL, const VR &>>{lhs, rhs};
}
template<class VL, class VR>
auto operator +(VectorExpr<VL> lhs, VectorExpr<VR> rhs) noexcept{
	SP_MATRIX_ERROR(
		len(lhs) != len(rhs),
		"added vectors cannot have different dimensions"
	);
	return VectorExpr<VectorAdd<VL, VR>>{lhs, rhs};
}




template<class VL, class VR>
auto operator -(const VectorBase<VL> &lhs, const VectorBase<VR> &rhs) noexcept{
	SP_MATRIX_ERROR(
		len(lhs) != len(rhs),
		"subtracted vectors cannot have different dimensions"
	);
	return VectorExpr<VectorSubtract<const VL &, const VR &>>{lhs, rhs};
}
template<class VL, class VR>
auto operator -(const VectorBase<VL> &lhs, VectorExpr<VR> rhs) noexcept{
	SP_MATRIX_ERROR(
		len(lhs) != len(rhs),
		"subtracted vectors cannot have different dimensions"
	);
	return VectorExpr<VectorSubtract<const VL &, VR>>{lhs, rhs};
}
template<class VL, class VR>
auto operator -(VectorExpr<VL> lhs, const VectorBase<VR> &rhs) noexcept{
	SP_MATRIX_ERROR(
		len(lhs) != len(rhs),
		"subtracted vectors cannot have different dimensions"
	);
	return VectorExpr<VectorSubtract<VL, const VR &>>{lhs, rhs};
}
template<class VL, class VR>
auto operator -(VectorExpr<VL> lhs, VectorExpr<VR> rhs) noexcept{
	SP_MATRIX_ERROR(
		len(lhs) != len(rhs),
		"subtracted vectors cannot have different dimensions"
	);
	return VectorExpr<VectorSubtract<VL, VR>>{lhs, rhs};
}




template<class V>
auto operator *(const VectorBase<V> &arg, typename std::decay_t<V>::ValueType scalar) noexcept{
	return VectorExpr<VectorScalarMultiply<const V &>>{arg, scalar};
}
template<class V>
auto operator *(VectorExpr<V> arg, typename std::decay_t<V>::ValueType scalar) noexcept{
	return VectorExpr<VectorScalarMultiply<V>>{arg, scalar};
}

template<class T, class V>
auto operator *(T scalar, const VectorBase<V> &arg) noexcept{
	return VectorExpr<VectorScalarMultiply<const V &>>{
		arg, (typename std::decay_t<V>::ValueType)scalar
	};
}
template<class T, class V>
auto operator *(T scalar, VectorExpr<V> arg) noexcept{
	return VectorExpr<VectorScalarMultiply<V>>{
		arg, (typename std::decay_t<V>::ValueType)scalar
	};
}




template<class V>
auto eladd(const VectorBase<V> &arg, typename std::decay_t<V>::ValueType scalar) noexcept{
	return VectorExpr<VectorScalarAdd<const V &>>{arg, scalar};
}
template<class V>
auto eladd(VectorExpr<V> arg, typename std::decay_t<V>::ValueType scalar) noexcept{
	return VectorExpr<VectorScalarAdd<V>>{arg, scalar};
}




template<class V>
auto elsub(const VectorBase<V> &arg, typename std::decay_t<V>::ValueType scalar) noexcept{
	return VectorExpr<VectorScalarSubtract<const V &>>{arg, -scalar};
}
template<class V>
auto elsub(VectorExpr<V> arg, typename std::decay_t<V>::ValueType scalar) noexcept{
	return VectorExpr<VectorScalarSubtract<V>>{arg, -scalar};
}

template<class T, class V>
auto elsub(T scalar, const VectorBase<V> &arg) noexcept{
	return VectorExpr<VectorScalarSubtract<const V &>>{
		arg, (typename std::decay_t<V>::ValueType)scalar
	};
}
template<class T, class V>
auto elsub(T scalar, VectorExpr<V> arg) noexcept{
	return VectorExpr<VectorScalarSubtract<V>>{
		arg, (typename std::decay_t<V>::ValueType)scalar
	};
}




template<class V>
auto operator /(const VectorBase<V> &arg, typename std::decay_t<V>::ValueType scalar) noexcept{
	return VectorExpr<VectorScalarMultiply<const V &>>{arg, Unit<decltype(scalar)>/scalar};
}
template<class V>
auto operator /(VectorExpr<V> arg, typename std::decay_t<V>::ValueType scalar) noexcept{
	return VectorExpr<VectorScalarMultiply<V>>{arg, Unit<decltype(scalar)>/scalar};
}












// MATRIX VECTOR EXPRESSIONS
template<class V>
struct MatrixVectorRow{
	typedef std::decay_t<V> Arg;

	typedef typename Arg::ValueType ValueType;
	constexpr static uint32_t Buffer = (Arg::Buffer != 0) << 1;
	constexpr static MatrixMajor Major = MatrixMajor::Undef;

	V arg;

	ValueType operator ()(size_t r, size_t c) const noexcept{
		return this->arg[c];
	}
};

template<class V>
size_t rows(MatrixVectorRow<V> v) noexcept{ return 1; }

template<class V>
size_t cols(MatrixVectorRow<V> v) noexcept{ return len(v.arg); }

template<class V>
size_t len(MatrixVectorRow<V> v) noexcept{ return len(v.arg); }




template<class V>
struct MatrixVectorColumn{
	typedef std::decay_t<V> Arg;

	typedef typename Arg::ValueType ValueType;
	constexpr static uint32_t Buffer = (Arg::Buffer != 0) << 1;
	constexpr static MatrixMajor Major = MatrixMajor::Undef;

	V arg;

	ValueType operator ()(size_t r, size_t c) const noexcept{
		return this->arg[r];
	}
};

template<class V>
size_t rows(MatrixVectorColumn<V> v) noexcept{ return len(v.arg); }

template<class V>
size_t cols(MatrixVectorColumn<V> v) noexcept{ return 1; }

template<class V>
size_t len(MatrixVectorColumn<V> v) noexcept{ return len(v.arg); }




template<class V>
struct MatrixVectorDiagonal{
	typedef std::decay_t<V> Arg;

	typedef typename Arg::ValueType ValueType;
	constexpr static uint32_t Buffer = (Arg::Buffer != 0) << 1;
	constexpr static MatrixMajor Major = MatrixMajor::Undef;

	V arg;

	ValueType operator ()(size_t r, size_t c) const noexcept{
		return r==c ? this->arg[r] : Null<ValueType>;
	}
};

template<class V>
size_t rows(MatrixVectorDiagonal<V> v) noexcept{ return len(v.arg); }

template<class V>
size_t cols(MatrixVectorDiagonal<V> v) noexcept{ return len(v.arg); }

template<class V>
size_t len(MatrixVectorDiagonal<V> v) noexcept{ return len(v.arg)*len(v.arg); }




template<class VL, class VR>
struct MatrixVectorOuterProduct{
	typedef std::decay_t<VL> Lhs;
	typedef std::decay_t<VR> Rhs;

	typedef typename Lhs::ValueType ValueType;
	constexpr static uint32_t Buffer = ((Lhs::Buffer != 0) << 3) | ((Rhs::Buffer != 0) << 2);
	constexpr static MatrixMajor Major = MatrixMajor::Undef;

	VL lhs;
	VR rhs;

	ValueType operator ()(size_t r, size_t c) const noexcept{
		return this->lhs[r] * this->rhs[c];
	}
};

template<class VL, class VR>
size_t rows(MatrixVectorOuterProduct<VL, VR> v) noexcept{ return len(v.lhs); }

template<class VL, class VR>
size_t cols(MatrixVectorOuterProduct<VL, VR> v) noexcept{ return len(v.rhs); }

template<class VL, class VR>
size_t len(MatrixVectorOuterProduct<VL, VR> v) noexcept{ return len(v.lhs)*len(v.rhs); }




template<class VL, class VR, auto Operation>
struct MatrixVectorOuterStatOp{
	typedef std::decay_t<VL> Lhs;
	typedef std::decay_t<VR> Rhs;

	typedef typename Lhs::ValueType ValueType;
	constexpr static uint32_t Buffer = ((Lhs::Buffer != 0) << 3) | ((Rhs::Buffer != 0) << 2);
	constexpr static MatrixMajor Major = MatrixMajor::Undef;

	VL lhs;
	VR rhs;

	ValueType operator ()(size_t r, size_t c) const noexcept{
		return Operation(this->lhs[r], this->rhs[c]);
	}
};

template<class VL, class VR, auto Op>
size_t rows(MatrixVectorOuterStatOp<VL, VR, Op> v) noexcept{ return len(v.lhs); }

template<class VL, class VR, auto Op>
size_t cols(MatrixVectorOuterStatOp<VL, VR, Op> v) noexcept{ return len(v.rhs); }

template<class VL, class VR, auto Op>
size_t len(MatrixVectorOuterStatOp<VL, VR, Op> v) noexcept{ return len(v.lhs)*len(v.rhs); }




template<class VL, class VR, class Operation>
struct MatrixVectorOuterDynOp{
	typedef std::decay_t<VL> Lhs;
	typedef std::decay_t<VR> Rhs;

	typedef typename Lhs::ValueType ValueType;
	constexpr static uint32_t Buffer = ((Lhs::Buffer != 0) << 3) | ((Rhs::Buffer != 0) << 2);
	constexpr static MatrixMajor Major = MatrixMajor::Undef;

	VL lhs;
	VR rhs;
	mutable Operation operation;

	ValueType operator ()(size_t r, size_t c) const noexcept{
		return operation(this->lhs[r], this->rhs[c]);
	}
};

template<class VL, class VR, class Op>
size_t rows(MatrixVectorOuterDynOp<VL, VR, Op> v) noexcept{ return len(v.lhs); }

template<class VL, class VR, class Op>
size_t cols(MatrixVectorOuterDynOp<VL, VR, Op> v) noexcept{ return len(v.rhs); }

template<class VL, class VR, class Op>
size_t len(MatrixVectorOuterDynOp<VL, VR, Op> v) noexcept{ return len(v.lhs)*len(v.rhs); }






// MATRIX VECTOR EXPRESSION OPERATIONS
template<class V>
auto row(const VectorBase<V> &arg) noexcept{
	return MatrixExpr<MatrixVectorRow<const V &>>{arg};
}
template<class V>
auto row(VectorExpr<V> arg) noexcept{
	return MatrixExpr<MatrixVectorRow<V>>{arg};
}




template<class V>
auto col(const VectorBase<V> &arg) noexcept{
	return MatrixExpr<MatrixVectorColumn<const V &>>{arg};
}
template<class V>
auto col(VectorExpr<V> arg) noexcept{
	return MatrixExpr<MatrixVectorColumn<V>>{arg};
}




template<class V>
auto diagonal(const VectorBase<V> &arg) noexcept{
	return MatrixExpr<MatrixVectorDiagonal<const V &>>{arg};
}
template<class V>
auto diagonal(VectorExpr<V> arg) noexcept{
	return MatrixExpr<MatrixVectorDiagonal<V>>{arg};
}



template<class VL, class VR>
auto outprod(const VectorBase<VL> &lhs, const VectorBase<VR> &rhs) noexcept{
	return MatrixExpr<MatrixVectorOuterProduct<const VL &, const VR &>>{lhs, rhs};
}
template<class VL, class VR>
auto outprod(const VectorBase<VL> &lhs, VectorExpr<VR> rhs) noexcept{
	return MatrixExpr<MatrixVectorOuterProduct<const VL &, VR>>{lhs, rhs};
}
template<class VL, class VR>
auto outprod(VectorExpr<VL> lhs, const VectorBase<VR> &rhs) noexcept{
	return MatrixExpr<MatrixVectorOuterProduct<VL, const VR &>>{lhs, rhs};
}
template<class VL, class VR>
auto outprod(VectorExpr<VL> lhs, VectorExpr<VR> rhs) noexcept{
	return MatrixExpr<MatrixVectorOuterProduct<VL, VR>>{lhs, rhs};
}



template<auto Op, class VL, class VR>
auto outop(const VectorBase<VL> &lhs, const VectorBase<VR> &rhs) noexcept{
	return MatrixExpr<MatrixVectorOuterStatOp<const VL &, const VR &, Op>>{lhs, rhs};
}
template<auto Op, class VL, class VR>
auto outop(const VectorBase<VL> &lhs, VectorExpr<VR> rhs) noexcept{
	return MatrixExpr<MatrixVectorOuterStatOp<const VL &, VR, Op>>{lhs, rhs};
}
template<auto Op, class VL, class VR>
auto outop(VectorExpr<VL> lhs, const VectorBase<VR> &rhs) noexcept{
	return MatrixExpr<MatrixVectorOuterStatOp<VL, const VR &, Op>>{lhs, rhs};
}
template<auto Op, class VL, class VR>
auto outop(VectorExpr<VL> lhs, VectorExpr<VR> rhs) noexcept{
	return MatrixExpr<MatrixVectorOuterStatOp<VL, VR, Op>>{lhs, rhs};
}



template<class VL, class VR, class Op>
auto outop(const VectorBase<VL> &lhs, const VectorBase<VR> &rhs, Op operation) noexcept{
	return MatrixExpr<MatrixVectorOuterDynOp<const VL &, const VR &, Op>>{lhs, rhs, operation};
}
template<class VL, class VR, class Op>
auto outop(const VectorBase<VL> &lhs, VectorExpr<VR> rhs, Op operation) noexcept{
	return MatrixExpr<MatrixVectorOuterDynOp<const VL &, VR, Op>>{lhs, rhs, operation};
}
template<class VL, class VR, class Op>
auto outop(VectorExpr<VL> lhs, const VectorBase<VR> &rhs, Op operation) noexcept{
	return MatrixExpr<MatrixVectorOuterDynOp<VL, const VR &, Op>>{lhs, rhs, operation};
}
template<class VL, class VR, class Op>
auto outop(VectorExpr<VL> lhs, VectorExpr<VR> rhs, Op operation) noexcept{
	return MatrixExpr<MatrixVectorOuterDynOp<VL, VR, Op>>{lhs, rhs, operation};
}












// VECTOR MATRIX EXPRESSIONS
template<class M>
struct VectorMatrixRow{
	typedef std::decay_t<M> Arg;

	typedef typename Arg::ValueType ValueType;
	constexpr static uint32_t Buffer = (Arg::Buffer != 0) << 1;

	M arg;
	uint32_t row;

	ValueType operator [](size_t c) const noexcept{
		return arg(this->row, c);
	}
};

template<class M>
size_t len(VectorMatrixRow<M> v) noexcept{ return cols(v.arg); }




template<class M>
struct VectorMatrixColumn{
	typedef std::decay_t<M> Arg;

	typedef typename Arg::ValueType ValueType;
	constexpr static uint32_t Buffer = (Arg::Buffer != 0) << 1;

	M arg;
	uint32_t col;

	ValueType operator [](size_t r) const noexcept{
		return arg(r, this->col);
	}
};

template<class M>
size_t len(VectorMatrixColumn<M> v) noexcept{ return rows(v.arg); }




template<class M>
struct VectorMatrixDiagonal{
	typedef std::decay_t<M> Arg;

	typedef typename Arg::ValueType ValueType;
	constexpr static uint32_t Buffer = (Arg::Buffer != 0) << 1;

	M arg;
	uint32_t col;

	ValueType operator [](size_t i) const noexcept{
		return arg(i, i);
	}
};

template<class M>
size_t len(VectorMatrixDiagonal<M> v) noexcept{ return min(rows(v.arg), cols(v.arg)); }




template<class VL, class MR>
struct VectorMatrixMultiplyLeft{
	typedef std::decay_t<VL> Lhs;
	typedef std::decay_t<MR> Rhs;

	typedef typename Lhs::ValueType ValueType;
	constexpr static uint32_t Buffer = ((Lhs::Buffer != 0) << 3) | ((Rhs::Buffer != 0) << 2);

	VL lhs;
	MR rhs;

	ValueType operator [](size_t c) const noexcept{
		ValueType res = Null<ValueType>;
		for (size_t i=0; i!=cols(lhs); ++i)
			res += lhs[i] * rhs(i, c);
		return res;
	}
};

template<class VL, class MR>
size_t len(VectorMatrixMultiplyLeft<VL, MR> v) noexcept{ return cols(v.rhs); }




template<class ML, class VR>
struct VectorMatrixMultiplyRight{
	typedef std::decay_t<ML> Lhs;
	typedef std::decay_t<VR> Rhs;

	typedef typename Lhs::ValueType ValueType;
	constexpr static uint32_t Buffer = ((Lhs::Buffer != 0) << 3) | ((Rhs::Buffer != 0) << 2);

	ML lhs;
	VR rhs;

	ValueType operator [](size_t r) const noexcept{
		ValueType res = Null<ValueType>;
		for (size_t i=0; i!=cols(lhs); ++i)
			res += rhs(r, i) * lhs[i];
		return res;
	}
};

template<class VL, class MR>
size_t len(VectorMatrixMultiplyRight<VL, MR> v) noexcept{ return rows(v.lhs); }






// VECTOR MATRIX EXPRESSION OPERATIONS
template<class M>
auto row(const MatrixBase<M> &arg, size_t row) noexcept{
	SP_MATRIX_ERROR(row >= rows(arg), "out of bounds matrix row");
	return VectorExpr<VectorMatrixRow<const M &>>{arg, row};
}
template<class M>
auto row(MatrixExpr<M> arg, size_t row) noexcept{
	SP_MATRIX_ERROR(row >= rows(arg), "out of bounds matrix row");
	return VectorExpr<VectorMatrixRow<M>>{arg, row};
}




template<class M>
auto col(const MatrixBase<M> &arg, size_t col) noexcept{
	SP_MATRIX_ERROR(col >= cols(arg), "out of bounds matrix column");
	return VectorExpr<VectorMatrixColumn<const M &>>{arg, col};
}
template<class M>
auto col(MatrixExpr<M> arg, size_t col) noexcept{
	SP_MATRIX_ERROR(col >= cols(arg), "out of bounds matrix column");
	return VectorExpr<VectorMatrixColumn<M>>{arg, col};
}




template<class M>
auto diagonal(const MatrixBase<M> &arg) noexcept{
	return VectorExpr<VectorMatrixDiagonal<const M &>>{arg};
}
template<class M>
auto diagonal(MatrixExpr<M> arg) noexcept{
	return VectorExpr<VectorMatrixDiagonal<M>>{arg};
}




template<class VL, class MR>
auto operator *(const VectorBase<VL> &lhs, const MatrixBase<MR> &rhs) noexcept{
	SP_MATRIX_ERROR(len(lhs) != rows(rhs), "multiplied vector and matrix have wrong dimensionss");
	return VectorExpr<VectorMatrixMultiplyLeft<const VL &, const MR &>>{lhs, rhs};
}
template<class VL, class MR>
auto operator *(const VectorBase<VL> &lhs, MatrixExpr<MR> rhs) noexcept{
	SP_MATRIX_ERROR(len(lhs) != rows(rhs), "multiplied vector and matrix have wrong dimensionss");
	return VectorExpr<VectorMatrixMultiplyLeft<const VL &, MR>>{lhs, rhs};
}
template<class VL, class MR>
auto operator *(VectorExpr<VL> lhs, const MatrixBase<MR> &rhs) noexcept{
	SP_MATRIX_ERROR(len(lhs) != rows(rhs), "multiplied vector and matrix have wrong dimensionss");
	return VectorExpr<VectorMatrixMultiplyLeft<VL, const MR &>>{lhs, rhs};
}
template<class VL, class MR>
auto operator *(VectorExpr<VL> lhs, MatrixExpr<MR> rhs) noexcept{
	SP_MATRIX_ERROR(len(lhs) != rows(rhs), "multiplied vector and matrix have wrong dimensionss");
	return VectorExpr<VectorMatrixMultiplyLeft<VL, MR>>{lhs, rhs};
}




template<class ML, class VR>
auto operator *(const MatrixBase<ML> &lhs, const VectorBase<VR> &rhs) noexcept{
	SP_MATRIX_ERROR(cols(lhs) != len(rhs), "multiplied matrix and vector have wrong dimensionss");
	return VectorExpr<VectorMatrixMultiplyRight<const ML &, const VR &>>{lhs, rhs};
}
template<class ML, class VR>
auto operator *(const MatrixBase<ML> &lhs, VectorExpr<VR> rhs) noexcept{
	SP_MATRIX_ERROR(cols(lhs) != len(rhs), "multiplied matrix and vector have wrong dimensionss");
	return VectorExpr<VectorMatrixMultiplyRight<const ML &, VR>>{lhs, rhs};
}
template<class ML, class VR>
auto operator *(MatrixExpr<ML> lhs, const VectorBase<VR> &rhs) noexcept{
	SP_MATRIX_ERROR(cols(lhs) != len(rhs), "multiplied matrix and vector have wrong dimensionss");
	return VectorExpr<VectorMatrixMultiplyRight<ML, const VR &>>{lhs, rhs};
}
template<class ML, class VR>
auto operator *(MatrixExpr<ML> lhs, VectorExpr<VR> rhs) noexcept{
	SP_MATRIX_ERROR(cols(lhs) != len(rhs), "multiplied matrix and vector have wrong dimensionss");
	return VectorExpr<VectorMatrixMultiplyRight<ML, VR>>{lhs, rhs};
}











// VALUE OPERATIONS
template<class ML, class MR>
bool operator ==(const MatrixBase<ML> &lhs, const MatrixBase<MR> &rhs) noexcept{
	if (rows(lhs)!=rows(rhs) || cols(lhs)!=cols(rhs)) return false;
	if constexpr (ML::Major == MatrixMajor::Row){
		for (size_t r=0; r!=rows(lhs); r+=1)
			for (size_t c=0; c!=cols(lhs); c+=1)
				if (lhs(r, c) != rhs(r, c)) return false;
	} else{
		for (size_t c=0; c!=cols(lhs); c+=1)
			for (size_t r=0; r!=rows(lhs); r+=1)
				if (lhs(r, c) != rhs(r, c)) return false;
	}
	return true;
}
template<class ML, class MR>
bool operator ==(const MatrixBase<ML> &lhs, MatrixExpr<MR> rhs) noexcept{
	if (rows(lhs)!=rows(rhs) || cols(lhs)!=cols(rhs)) return false;
	if constexpr (ML::Major == MatrixMajor::Row){
		for (size_t r=0; r!=rows(lhs); r+=1)
			for (size_t c=0; c!=cols(lhs); c+=1)
				if (lhs(r, c) != rhs(r, c)) return false;
	} else{
		for (size_t c=0; c!=cols(lhs); c+=1)
			for (size_t r=0; r!=rows(lhs); r+=1)
				if (lhs(r, c) != rhs(r, c)) return false;
	}
	return true;
}
template<class ML, class MR>
bool operator ==(MatrixExpr<ML> lhs, const MatrixBase<MR> &rhs) noexcept{
	if (rows(lhs)!=rows(rhs) || cols(lhs)!=cols(rhs)) return false;
	if constexpr (ML::Major == MatrixMajor::Row){
		for (size_t r=0; r!=rows(lhs); r+=1)
			for (size_t c=0; c!=cols(lhs); c+=1)
				if (lhs(r, c) != rhs(r, c)) return false;
	} else{
		for (size_t c=0; c!=cols(lhs); c+=1)
			for (size_t r=0; r!=rows(lhs); r+=1)
				if (lhs(r, c) != rhs(r, c)) return false;
	}
	return true;
}
template<class ML, class MR>
bool operator ==(MatrixExpr<ML> lhs, MatrixExpr<MR> rhs) noexcept{
	if (rows(lhs)!=rows(rhs) || cols(lhs)!=cols(rhs)) return false;
	if constexpr (ML::Major == MatrixMajor::Row){
		for (size_t r=0; r!=rows(lhs); r+=1)
			for (size_t c=0; c!=cols(lhs); c+=1)
				if (lhs(r, c) != rhs(r, c)) return false;
	} else{
		for (size_t c=0; c!=cols(lhs); c+=1)
			for (size_t r=0; r!=rows(lhs); r+=1)
				if (lhs(r, c) != rhs(r, c)) return false;
	}
	return true;
}




template<class ML, class MR>
bool operator !=(const MatrixBase<ML> &lhs, const MatrixBase<MR> &rhs) noexcept{
	return !(lhs == rhs);
}
template<class ML, class MR>
bool operator !=(const MatrixBase<ML> &lhs, MatrixExpr<MR> rhs) noexcept{
	return !(lhs == rhs);
}
template<class ML, class MR>
bool operator !=(MatrixExpr<ML> lhs, const MatrixBase<MR> &rhs) noexcept{
	return !(lhs == rhs);
}
template<class ML, class MR>
bool operator !=(MatrixExpr<ML> lhs, MatrixExpr<MR> rhs) noexcept{
	return !(lhs == rhs);
}




template<class ML, class MR>
bool operator ==(const VectorBase<ML> &lhs, const VectorBase<MR> &rhs) noexcept{
	if (len(lhs) != len(rhs)) return false;
	for (size_t i=0; i!=len(lhs); i+=1)
		if (lhs[i] != rhs[i]) return false;
	return true;
}
template<class ML, class MR>
bool operator ==(const VectorBase<ML> &lhs, VectorExpr<MR> rhs) noexcept{
	if (len(lhs) != len(rhs)) return false;
	for (size_t i=0; i!=len(lhs); i+=1)
		if (lhs[i] != rhs[i]) return false;
	return true;
}
template<class ML, class MR>
bool operator ==(VectorExpr<ML> lhs, const VectorBase<MR> &rhs) noexcept{
	if (len(lhs) != len(rhs)) return false;
	for (size_t i=0; i!=len(lhs); i+=1)
		if (lhs[i] != rhs[i]) return false;
	return true;
}
template<class ML, class MR>
bool operator ==(VectorExpr<ML> lhs, VectorExpr<MR> rhs) noexcept{
	if (len(lhs) != len(rhs)) return false;
	for (size_t i=0; i!=len(lhs); i+=1)
		if (lhs[i] != rhs[i]) return false;
	return true;
}




template<class M>
bool is_lower_triangular(
	const MatrixBase<M> &arg,
	typename M::ValueType eps = Null<typename M::ValueType>
) noexcept{
	if constexpr (M::Major != MatrixMajor::Col){
		for (size_t r=1; r<rows(arg); r+=1)
			for (size_t c=r+1; c<cols(arg); c+=1)
				if (eps < arg(r, c)) return false;
	} else{
		for (size_t c=1; c<cols(arg); c+=1)
			for (size_t r=0; r<c; r+=1)
				if (eps < arg(r, c)) return false;
	}
	return true;
}
template<class M>
bool is_lower_triangular(
	MatrixExpr<M> arg,
	typename M::ValueType eps = Null<typename M::ValueType>
) noexcept{
	if constexpr (M::Major != MatrixMajor::Col){
		for (size_t r=1; r<rows(arg); r+=1)
			for (size_t c=r+1; c<cols(arg); c+=1)
				if (eps < arg(r, c)) return false;
	} else{
		for (size_t c=1; c<cols(arg); c+=1)
			for (size_t r=0; r<c; r+=1)
				if (eps < arg(r, c)) return false;
	}
	return true;
}




template<class M>
bool is_upper_triangular(
	const MatrixBase<M> &arg,
	typename M::ValueType eps = Null<typename M::ValueType>
) noexcept{
	if constexpr (M::Major != MatrixMajor::Col){
		for (size_t r=1; r<rows(arg); r+=1)
			for (size_t c=0; c<r; c+=1)
				if (eps < arg(r, c)) return false;
	} else{
		for (size_t c=1; c<cols(arg); c+=1)
			for (size_t r=c+1; r<rows(arg); r+=1)
				if (eps < arg(r, c)) return false;
	}
	return true;
}
template<class M>
bool is_upper_triangular(
	MatrixExpr<M> arg,
	typename M::ValueType eps = Null<typename M::ValueType>
) noexcept{
	if constexpr (M::Major != MatrixMajor::Col){
		for (size_t r=1; r<rows(arg); r+=1)
			for (size_t c=0; c<r; c+=1)
				if (eps < arg(r, c)) return false;
	} else{
		for (size_t c=1; c<cols(arg); c+=1)
			for (size_t r=c+1; r<rows(arg); r+=1)
				if (eps < arg(r, c)) return false;
	}
	return true;
}




template<class M>
bool is_symmetric(
	const MatrixBase<M> &arg,
	typename M::ValueType eps = Null<typename M::ValueType>
) noexcept{
	SP_MATRIX_ERROR(rows(arg) != cols(arg), "only square matrix can be tested for being symmetric");
	for (size_t r=1; r<rows(arg); r+=1)
		for (size_t c=0; c<r; c+=1)
			if (eps < abs(arg(r, c) - arg(c, r))) return false;
	return true;
}
template<class M>
bool is_symmetric(
	MatrixExpr<M> arg,
	typename M::ValueType eps = Null<typename M::ValueType>
) noexcept{
	SP_MATRIX_ERROR(rows(arg) != cols(arg), "only square matrix can be tested for being symmetric");
	for (size_t r=1; r<rows(arg); r+=1)
		for (size_t c=0; c<r; c+=1)
			if (eps < abs(arg(r, c) - arg(c, r))) return false;
	return true;
}




template<class M>
bool is_diagonal(
	const MatrixBase<M> &arg,
	typename M::ValueType eps = Null<typename M::ValueType>
) noexcept{
	bool lower = false;
	bool higher = false;
	if constexpr (M::Major != MatrixMajor::Col){
		for (size_t r=1; r<rows(arg); r+=1){
			for (size_t c=0; c<r; c+=1) lower |= eps < arg(r, c);
			for (size_t c=r+1; c<cols(arg); c+=1) higher |= eps < arg(r, c);
			if (lower | higher) return false;
		}
	} else{
		for (size_t c=1; c!=cols(arg); c+=1){
			for (size_t r=0; r<c; r+=1) higher |= eps < arg(r, c);
			for (size_t r=r+1; r<rows(arg); r+=1) lower |= eps < arg(r, c);
			if (lower | higher) return false;
		}
	}
	return true;
}
template<class M>
bool is_diagonal(
	MatrixExpr<M> arg,
	typename M::ValueType eps = Null<typename M::ValueType>
) noexcept{
	bool lower = false;
	bool higher = false;
	if constexpr (M::Major != MatrixMajor::Col){
		for (size_t r=1; r<rows(arg); r+=1){
			for (size_t c=0; c<r; c+=1) lower |= eps < arg(r, c);
			for (size_t c=r+1; c<cols(arg); c+=1) higher |= eps < arg(r, c);
			if (lower | higher) return false;
		}
	} else{
		for (size_t c=1; c!=cols(arg); c+=1){
			for (size_t r=0; r<c; r+=1) higher |= eps < arg(r, c);
			for (size_t r=r+1; r<rows(arg); r+=1) lower |= eps < arg(r, c);
			if (lower | higher) return false;
		}
	}
	return true;
}




template<class VL, class VR>
bool operator !=(const VectorBase<VL> &lhs, const VectorBase<VR> &rhs) noexcept{
	return !(lhs == rhs);
}
template<class VL, class VR>
bool operator !=(const VectorBase<VL> &lhs, VectorExpr<VR> rhs) noexcept{
	return !(lhs == rhs);
}
template<class VL, class VR>
bool operator !=(VectorExpr<VL> lhs, const VectorBase<VR> &rhs) noexcept{
	return !(lhs == rhs);
}
template<class VL, class VR>
bool operator !=(VectorExpr<VL> lhs, VectorExpr<VR> rhs) noexcept{
	return !(lhs == rhs);
}



template<class VL, class VR>
auto operator *(const VectorBase<VL> &lhs, const VectorBase<VR> &rhs) noexcept{
	SP_MATRIX_ERROR(
		len(lhs) != len(rhs),
		"vector arguments for inner product have different sizes"
	);
	typename VL::ValueType res = Null<typename VL::ValueType>;
	for (size_t i=0; i!=len(lhs); i+=1)
		res += lhs[i] * rhs[i];
	return res;
}
template<class VL, class VR>
auto operator *(const VectorBase<VL> &lhs, VectorExpr<VR> rhs) noexcept{
	SP_MATRIX_ERROR(
		len(lhs) != len(rhs),
		"vector arguments for inner product have different sizes"
	);
	typename VL::ValueType res = Null<typename VL::ValueType>;
	for (size_t i=0; i!=len(lhs); i+=1)
		res += lhs[i] * rhs[i];
	return res;
}
template<class VL, class VR>
auto operator *(VectorExpr<VL> lhs, const VectorBase<VR> &rhs) noexcept{
	SP_MATRIX_ERROR(
		len(lhs) != len(rhs),
		"vector arguments for inner product have different sizes"
	);
	typename VL::ValueType res = Null<typename VL::ValueType>;
	for (size_t i=0; i!=len(lhs); i+=1)
		res += lhs[i] * rhs[i];
	return res;
}
template<class VL, class VR>
auto operator *(VectorExpr<VL> lhs, VectorExpr<VR> rhs) noexcept{
	SP_MATRIX_ERROR(
		len(lhs) != len(rhs),
		"vector arguments for inner product have different sizes"
	);
	typename VL::ValueType res = Null<typename VL::ValueType>;
	for (size_t i=0; i!=len(lhs); i+=1)
		res += lhs[i] * rhs[i];
	return res;
}




template<class M>
auto trace(const MatrixBase<M> &arg) noexcept{
	size_t len = min(rows(arg), cols(arg));
	typename M::ValueType res = Null<typename M::ValueType>;
	for (size_t i=0; i!=len; i+=1) res += arg(i, i);
	return res;
}
template<class M>
auto trace(MatrixExpr<M> arg) noexcept{
	size_t len = min(rows(arg), cols(arg));
	typename M::ValueType res = Null<typename M::ValueType>;
	for (size_t i=0; i!=len; i+=1) res += arg(i, i);
	return res;
}



/*
template<SP_MATRIX_T(M)>
auto determinant(M &&A) noexcept{
	SP_MATRIX_ERROR(rows(A) != cols(A), "only square matrix can have a determinant");
	size_t length = rows(A);
	
	if constexpr (std::is_rvalue_reference_v<M>){
		typename std::decay_t<M>::ValueType result = typename std::decay_t<M>::ValueType{1};
		typename std::decay_t<M>::ValueType factor1, factor2;
		if constexpr (std::decay_t<M>::RowMajor){
			for (size_t i=0; i<length-1; ++i){
				{
					size_t j = i;
					for (size_t k=i+1; k!=length; ++k)	// find row with max value
						j = abs(A(k, i))>abs(A(j, i)) ? k : j;
					if (j != i){
						for (size_t k=i; k!=length; ++k)	// exchange top row with row with max value
							swap(A(i, k), A(j, k));
						result = -result;
					}
				}
				factor1 = A(i, i);
				result *= factor1;
				for (size_t j=i+1; j!=length; ++j){
					factor2 = A(j, i) / factor1;
					for (size_t k=i+1; k!=length; ++k)
						A(j, k) -= A(i, k) * factor2;
				}
			}
		} else{
			for (size_t i=0; i<length-1; ++i){
				{
					size_t j = i;
					for (size_t k=i+1; k!=length; ++k)	// find row with max value
						j = abs(A(i, k))>abs(A(i, j)) ? k : j;
					if (j != i){
						for (size_t k=i; k!=length; ++k)	// exchange top row with row with max value
							swap(A(k, i), A(k, j));
						result = -result;
					}
				}
				factor1 = A(i, i);
				result *= factor1;
				for (size_t j=i+1; j!=length; ++j){
					factor2 = A(j, i) / factor1;
					for (size_t k=i+1; k!=length; ++k)
						A(k, j) -= A(k, i) * factor2;
				}
			}
		}
		return result * A(length-1, length-1);
	} else{
		size_t oldSize = len(MatrixTempStorage.data);
		expand_back(
			MatrixTempStorage.data,
			(length*length * sizeof(typename std::decay_t<M>::ValueType) + 7) / 8
		);
		typename std::decay_t<M>::ValueType *TempStorage = (
			typename std::decay_t<M>::ValueType *
		)beg(MatrixTempStorage.data) + oldSize;

		{
			typename std::decay_t<M>::ValueType *I = TempStorage;
			for (size_t i=0; i!=length; ++i)
				for (size_t j=0; j!=length; ++j, ++I)
					if constexpr (std::decay_t<M>::RowMajor)
						*I = A(i, j);
					else
						*I = A(j, i);
		}
		
		typename std::decay_t<M>::ValueType result = typename std::decay_t<M>::ValueType{1};
		typename std::decay_t<M>::ValueType factor1, factor2;
		for (size_t i=0; i<length-1; ++i){
			{
				size_t j = i;
				for (size_t k=i+1; k!=length; ++k)	// find row with max value
					j = abs(TempStorage[k*length+i])>abs(TempStorage[j*length+i]) ? k : j;
				if (j != i){
					for (size_t k=i; k!=length; ++k)	// exchange top row with row with max value
						swap(TempStorage[i*length+k], TempStorage[j*length+k]);
					result = -result;
				}
			}
			factor1 = TempStorage[i*(length+1)];
			result *= factor1;
			for (size_t j=i+1; j!=length; ++j){
				factor2 = TempStorage[j*length+i] / factor1;
				for (size_t k=i+1; k!=length; ++k)
					TempStorage[j*length+k] -= TempStorage[i*length+k] * factor2;
			}
		}
		result *= TempStorage[length*length-1];
		
		resize(MatrixTempStorage.data, oldSize);
		return result;
	}
}
*/


























// USER FRIENDLY ALIASES
template<class T, size_t R, size_t C = R, size_t RowMajor = true>
using FixedMatrix = MatrixBase<MatrixFixed<T, R, C, RowMajor>>;

template<class T, size_t C, size_t RowMajor = true>
using FiniteMatrix = MatrixBase<MatrixFinite<T, C, RowMajor>>;

template<class T, auto A = &matrix_allocator, size_t RowMajor = true>
using DynamicMatrix = MatrixBase<MatrixDynamic<T, A, RowMajor>>;


template<class T, size_t S>
using FixedVector = VectorBase<VectorFixed<T, S>>;

template<class T, size_t C>
using FiniteVector = VectorBase<VectorFinite<T, C>>;

template<class T, auto A = &matrix_allocator>
using DynamicVector = VectorBase<VectorDynamic<T, A>>;
