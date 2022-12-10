#include "SPL/Utils.hpp"


#ifdef SP_DEBUG
#define SP_ASSERT(cond, message) \
	if (!(cond)){ \
		fputs(message, stderr); \
		putc('\n', stderr); \
		exit(1); \
	}
#else
#define SP_ASSERT(cond, message)
#endif








// MATRIX WRAPPER
template<class M>
struct MatrixBase : M{
	template<class MR>
	M &operator =(const MatrixBase<MR> &rhs) noexcept{
		resize(*this, rows(rhs), cols(rhs));
		for (size_t r=0; r!=rows(rhs); ++r)
			for (size_t c=0; c!=cols(rhs); ++c)
				(*this)(r, c) = rhs(r, c);
		return *this;
	}
	
	template<class MR>
	M &operator +=(const MatrixBase<MR> &rhs) noexcept{
		resize(*this, rows(rhs), cols(rhs));
		for (size_t r=0; r!=rows(rhs); ++r)
			for (size_t c=0; c!=cols(rhs); ++c)
				(*this)(r, c) += rhs(r, c);
		return *this;
	}
	
	template<class MR>
	M &operator -=(const MatrixBase<MR> &rhs) noexcept{
		resize(*this, rows(rhs), cols(rhs));
		for (size_t r=0; r!=rows(rhs); ++r)
			for (size_t c=0; c!=cols(rhs); ++c)
				(*this)(r, c) -= rhs(r, c);
		return *this;
	}
	
	template<class MR>
	M &operator *=(const MatrixBase<MR> &rhs) noexcept{
		resize(*this, rows(*this), cols(rhs));
		SP_ASSERT(false, "matrix assign multiply is unimplemented");
		return *this;
	}
	
	M &operator *=(decltype((*this)(0, 0)) rhs) noexcept{
		resize(*this, rows(rhs), cols(rhs));
		for (size_t r=0; r!=rows(rhs); ++r)
			for (size_t c=0; c!=cols(rhs); ++c)
				(*this)(r, c) *= rhs;
		return *this;
	}
	
	M &operator /=(decltype((*this)(0, 0)) rhs) noexcept{
		resize(*this, rows(rhs), cols(rhs));
		for (size_t r=0; r!=rows(rhs); ++r)
			for (size_t c=0; c!=cols(rhs); ++c)
				(*this)(r, c) /= rhs;
		return *this;
	}
};

//namespace sp{



// CONTAINER MATRIXES
template<class T, size_t R, size_t C>
struct MatrixFixed{
	static constexpr bool UsesBuffer = false;

	T &operator ()(size_t r, size_t c) noexcept{
		SP_ASSERT(r*C+c < R*C, "index out of bounds");
		return *((T *)data + r*C + c);
	}
	const T &operator ()(size_t r, size_t c) const noexcept{
		SP_ASSERT(r*C+c < R*C, "index out of bounds");
		return *((const T *)data + r*C + c);
	}
	
	T data[R][C];
};

template<class T, size_t R, size_t C>
size_t rows(const MatrixFixed<T, R, C> &) noexcept{ return R; }
template<class T, size_t R, size_t C>
size_t cols(const MatrixFixed<T, R, C> &) noexcept{ return C; }
template<class T, size_t R, size_t C>
size_t len(const MatrixFixed<T, R, C> &) noexcept{ return R*C; }

template<class T, size_t R, size_t C>
bool resize(MatrixFixed<T, R, C> &, size_t r, size_t c) noexcept{
	SP_ASSERT(r == R && c == C, "fixed matrix is getting assigned to wrong dimensions");
	return false;
}


template<class T, size_t C>
struct MatrixFinite{
	static constexpr bool UsesBuffer = false;
	
	T &operator ()(size_t r, size_t c) noexcept{
		SP_ASSERT(r*this->cols+c < this->rows*this->cols, "index out of bounds");
		return *((T *)data + r*(size_t)this->cols + c);
	}
	const T &operator ()(size_t r, size_t c) const noexcept{
		SP_ASSERT(r*this->cols+c < this->rows*this->cols, "index out of bounds");
		return *((const T *)data + r*(size_t)this->cols + c);
	}
	
	T data[C];
	uint32_t rows;
	uint32_t cols;
};

template<class T, size_t C>
size_t rows(const MatrixFinite<T, C> &m) noexcept{ return m.rows; }
template<class T, size_t C>
size_t cols(const MatrixFinite<T, C> &m) noexcept{ return m.cols; }
template<class T, size_t C>
size_t len(const MatrixFinite<T, C> &m) noexcept{ return (size_t)m.rows*(size_t)m.cols; }
template<class T, size_t C>
size_t cap(const MatrixFinite<T, C> &) noexcept{ return C; }

template<class T, size_t C>
bool resize(MatrixFinite<T, C> &m, size_t r, size_t c) noexcept{
	SP_ASSERT(r*c < C, "finite matrix has not enough capacity");
	m.rows = r;
	m.cols = c;
	return false;
}



template<class T, auto A>
struct MatrixDynamic{
	static constexpr bool UsesBuffer = false;
	
	T &operator ()(size_t r, size_t c) noexcept{
		SP_ASSERT(r*this->cols+c < this->rows*this->cols, "index out of bounds");
		return *((T *)data + r*(size_t)this->cols + c);
	}
	const T &operator ()(size_t r, size_t c) const noexcept{
		SP_ASSERT(r*this->cols+c < this->rows*this->cols, "index out of bounds");
		return *((const T *)data + r*(size_t)this->cols + c);
	}
	
	T *data;
	uint32_t rows;
	uint32_t cols;
};

template<class T, size_t C>
size_t rows(const MatrixFinite<T, C> &m) noexcept{ return m.rows; }
template<class T, size_t C>
size_t cols(const MatrixFinite<T, C> &m) noexcept{ return m.cols; }
template<class T, size_t C>
size_t len(const MatrixFinite<T, C> &m) noexcept{ return (size_t)m.rows*(size_t)m.cols; }
template<class T, size_t C>
size_t cap(const MatrixFinite<T, C> &) noexcept{ return C; }

template<class T, size_t C>
bool resize(MatrixFinite<T, C> &m, size_t r, size_t c) noexcept{
	SP_ASSERT(r*c < C, "finite matrix has not enough capacity");
	m.rows = r;
	m.cols = c;
	return false;
}




// L VALUE EXPRESSION MATRIES
template<class T>
struct MatrixSpan{
	static constexpr bool UsesBuffer = false;
	
	T &operator ()(size_t r, size_t c) noexcept{
		SP_ASSERT(r*this->cols+c < this->rows*this->cols, "index out of bounds");
		return *(data + r*(size_t)this->cols + c);
	}
	const T &operator ()(size_t r, size_t c) const noexcept{
		SP_ASSERT(r*this->cols+c < this->rows*this->cols, "index out of bounds");
		return *(data + r*(size_t)this->cols + c);
	}

	T *data;
	uint32_t rows;
	uint32_t cols;
};

template<class T>
size_t rows(const MatrixSpan<T> &m) noexcept{ return m.rows; }
template<class T>
size_t cols(const MatrixSpan<T> &m) noexcept{ return m.cols; }
template<class T>
size_t len(const MatrixSpan<T> &m) noexcept{ return (size_t)m.rows*(size_t)m.cols; }

template<class T>
bool resize(MatrixSpan<T> &m, size_t r, size_t c) noexcept{
	SP_ASSERT(
		r == m.rows && c == m.cols,
		"matrix span is getting assigned to wrong dimensions"
	);
	return false;
}



template<class M>
struct MatrixWrapTranspose{
	static constexpr bool UsesBuffer = M::UsesBuffer;
	
	decltype(auto) operator ()(size_t r, size_t c) const noexcept{ return this->m(c, r); }

	M &m;
};

template<class M>
size_t rows(const MatrixWrapTranspose<M> &m) noexcept{ return cols(m.m); }
template<class M>
size_t cols(const MatrixWrapTranspose<M> &m) noexcept{ return rows(m.m); }
template<class M>
size_t len(const MatrixWrapTranspose<M> &m) noexcept{ return len(m.m); }
template<class M>
size_t cap(const MatrixWrapTranspose<M> &m) noexcept{ return cap(m.m); }

template<class M>
bool resize(MatrixWrapTranspose<M> &m, size_t r, size_t c) noexcept{
	return resize(m.m, c, r);
}

template<class M>
auto tr_wrap(MatrixBase<M> &m) noexcept{ return MatrixBase<MatrixWrapTranspose<M>>{{m}}; }



// R VALUE EXPRESSION MATRIXES
template<class M>
struct MatrixTranspose{
	static constexpr bool UsesBuffer = M::UsesBuffer;
	
	auto operator ()(size_t r, size_t c) const noexcept{ return this->m(c, r); }

	const M &m;
};

template<class M>
size_t rows(const MatrixTranspose<M> &m) noexcept{ return cols(m.m); }
template<class M>
size_t cols(const MatrixTranspose<M> &m) noexcept{ return rows(m.m); }
template<class M>
size_t len(const MatrixTranspose<M> &m) noexcept{ return len(m.m); }

template<class M>
auto tr(const MatrixBase<M> &m) noexcept{ return MatrixBase<MatrixTranspose<M>>{{m}}; }
template<class ML, class MR>



struct MatrixSum{
	static constexpr bool UsesBuffer = ML::UsesBuffer || MR::UsesBuffer;
	
	auto operator ()(size_t r, size_t c) const noexcept{ return this->lhs(r, c) + this->rhs(r, c); }

	const ML &lhs;
	const MR &rhs;
};

template<class ML, class MR>
size_t rows(const MatrixSum<ML, MR> &m) noexcept{ return rows(m.lhs); }
template<class ML, class MR>
size_t cols(const MatrixSum<ML, MR> &m) noexcept{ return cols(m.lhs); }
template<class ML, class MR>
size_t len(const MatrixSum<ML, MR> &m) noexcept{ return len(m.lhs); }

template<class ML, class MR>
auto operator +(const MatrixBase<ML> &lhs, const MatrixBase<MR> &rhs) noexcept{
	SP_ASSERT(
		rows(lhs)==rows(rhs) && cols(lhs)==cols(rhs),
		"dimensions of added matrixes does not match"
	);
	return MatrixBase<MatrixSum<ML, MR>>{{lhs, rhs}};
}



template<class ML, class MR>
struct MatrixDifference{
	static constexpr bool UsesBuffer = ML::UsesBuffer || MR::UsesBuffer;
	
	auto operator ()(size_t r, size_t c) const noexcept{ return this->lhs(r, c) - this->rhs(r, c); }

	const ML &lhs;
	const MR &rhs;
};

template<class ML, class MR>
size_t rows(const MatrixDifference<ML, MR> &m) noexcept{ return rows(m.lhs); }
template<class ML, class MR>
size_t cols(const MatrixDifference<ML, MR> &m) noexcept{ return cols(m.lhs); }
template<class ML, class MR>
size_t len(const MatrixDifference<ML, MR> &m) noexcept{ return len(m.lhs); }

template<class ML, class MR>
auto operator -(const MatrixBase<ML> &lhs, const MatrixBase<MR> &rhs) noexcept{
	SP_ASSERT(
		rows(lhs)==rows(rhs) && cols(lhs)==cols(rhs),
		"dimensions of subtracted matrixes does not match"
	);
	return MatrixBase<MatrixDifference<ML, MR>>{{lhs, rhs}};
}



template<class ML, class MR>
struct MatrixElwiseProduct{
	static constexpr bool UsesBuffer = ML::UsesBuffer || MR::UsesBuffer;
	
	auto operator ()(size_t r, size_t c) const noexcept{ return this->lhs(r, c) * this->rhs(r, c); }

	const ML &lhs;
	const MR &rhs;
};

template<class ML, class MR>
size_t rows(const MatrixElwiseProduct<ML, MR> &m) noexcept{ return rows(m.lhs); }
template<class ML, class MR>
size_t cols(const MatrixElwiseProduct<ML, MR> &m) noexcept{ return cols(m.lhs); }
template<class ML, class MR>
size_t len(const MatrixElwiseProduct<ML, MR> &m) noexcept{ return len(m.lhs); }

template<class ML, class MR>
auto elmul(const MatrixBase<ML> &lhs, const MatrixBase<MR> &rhs) noexcept{
	SP_ASSERT(
		rows(lhs)==rows(rhs) && cols(lhs)==cols(rhs),
		"dimensions of element wise multiplied matrixes does not match"
	);
	return MatrixBase<MatrixElwiseProduct<ML, MR>>{{lhs, rhs}};
}



template<class ML, class MR>
struct MatrixElwiseDivision{
	static constexpr bool UsesBuffer = ML::UsesBuffer || MR::UsesBuffer;
	
	auto operator ()(size_t r, size_t c) const noexcept{ return this->lhs(r, c) / this->rhs(r, c); }

	const ML &lhs;
	const MR &rhs;
};

template<class ML, class MR>
size_t rows(const MatrixElwiseDivision<ML, MR> &m) noexcept{ return rows(m.lhs); }
template<class ML, class MR>
size_t cols(const MatrixElwiseDivision<ML, MR> &m) noexcept{ return cols(m.lhs); }
template<class ML, class MR>
size_t len(const MatrixElwiseDivision<ML, MR> &m) noexcept{ return len(m.lhs); }

template<class ML, class MR>
auto eldiv(const MatrixBase<ML> &lhs, const MatrixBase<MR> &rhs) noexcept{
	SP_ASSERT(
		rows(lhs)==rows(rhs) && cols(lhs)==cols(rhs),
		"dimensions of element wise divided matrixes does not match"
	);
	return MatrixBase<MatrixElwiseDivision<ML, MR>>{{lhs, rhs}};
}


template<class ML, class MR, auto oper>
struct MatrixApplyStatic{
	static constexpr bool UsesBuffer = ML::UsesBuffer || MR::UsesBuffer;
	
	auto operator ()(size_t r, size_t c) const noexcept{
		return oper(this->lhs(r, c), this->rhs(r, c));
	}

	const ML &lhs;
	const MR &rhs;
};

template<class ML, class MR>
size_t rows(const MatrixApplyStatic<ML, MR> &m) noexcept{ return rows(m.lhs); }
template<class ML, class MR>
size_t cols(const MatrixApplyStatic<ML, MR> &m) noexcept{ return cols(m.lhs); }
template<class ML, class MR>
size_t len(const MatrixApplyStatic<ML, MR> &m) noexcept{ return len(m.lhs); }

template<auto O, class ML, class MR>
auto apply(const MatrixBase<ML> &lhs, const MatrixBase<MR> &rhs) noexcept{
	SP_ASSERT(
		rows(lhs)==rows(rhs) && cols(lhs)==cols(rhs),
		"dimensions of element wise operated matrixes does not match"
	);
	return MatrixBase<MatrixApply<ML, MR, O>>{{lhs, rhs}};
}



template<class ML, class MR, Class Oper>
struct MatrixApplyDynamic{
	static constexpr bool UsesBuffer = ML::UsesBuffer || MR::UsesBuffer;
	
	auto operator ()(size_t r, size_t c) const noexcept{
		return this->oper(this->lhs(r, c), this->rhs(r, c));
	}

	const ML &lhs;
	const MR &rhs;
	Oper oper;
};

template<class ML, class MR>
size_t rows(const MatrixApplyDynamic<ML, MR> &m) noexcept{ return rows(m.lhs); }
template<class ML, class MR>
size_t cols(const MatrixApplyDynamic<ML, MR> &m) noexcept{ return cols(m.lhs); }
template<class ML, class MR>
size_t len(const MatrixApplyDynamic<ML, MR> &m) noexcept{ return len(m.lhs); }

template<class ML, class MR, class Oper>
auto apply(const MatrixBase<ML> &lhs, const MatrixBase<MR> &rhs, const Oper &oper) noexcept{
	SP_ASSERT(
		rows(lhs)==rows(rhs) && cols(lhs)==cols(rhs),
		"dimensions of element wise operated matrixes does not match"
	);
	return MatrixBase<MatrixApply<ML, MR, Oper>>{{lhs, rhs, oper}};
}



template<class ML, class MR>
struct MatrixProduct{
	static constexpr bool UsesBuffer = ML::UsesBuffer || MR::UsesBuffer;
	
	auto operator ()(size_t r, size_t c) const noexcept{
		auto sum = decltype(this->lhs(0, 0)){};
		for (size_t i=0; i!=cols(lhs); ++i)
			sum += this->lhs(r, i) * this->rhs(i, c);
		return sum;
	}

	const ML &lhs;
	const MR &rhs;
};

template<class ML, class MR>
size_t rows(const MatrixProduct<ML, MR> &m) noexcept{ return rows(m.lhs); }
template<class ML, class MR>
size_t cols(const MatrixProduct<ML, MR> &m) noexcept{ return cols(m.rhs); }
template<class ML, class MR>
size_t len(const MatrixProduct<ML, MR> &m) noexcept{ return rows(m.lhs) * cols(m.rhs); }

template<class ML, class MR>
auto operator *(const MatrixBase<ML> &lhs, const MatrixBase<MR> &rhs) noexcept{
	SP_ASSERT(
		cols(lhs)==rows(rhs),
		"wrong dimensions for multiplied matrixes"
	);
	return MatrixBase<MatrixProduct<ML, MR>>{{lhs, rhs}};
}
//} // END OF NAMESPACE ///////////////////////////////////////////////////////////////////////////
