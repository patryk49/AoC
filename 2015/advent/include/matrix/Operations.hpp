#pragma once

#include "Expr.hpp"


namespace sp{

template<SP_MATRIX_T(M1), SP_MATRIX_T(M2)>
void transpose(M1 &&dest, M2 &&A) noexcept{
	dest.resize(A.cols(), A.rows());
	const size_t blockSize = CacheBlockLen / sizeof(A(0, 0));
	
	for (size_t i=0; i<rows(dest); i+=blockSize)
		for (size_t j=0; j<cols(dest); j+=blockSize)
			for (size_t ii=i; ii<i+blockSize; ++ii)
				for (size_t jj=j; jj<j+blockSize; ++jj)
					dest(ii, jj) = A(jj, ii);
}

template<SP_MATRIX_T(M1), SP_MATRIX_T(M2), SP_MATRIX_T(M3)>
void kronProduct(M1 &&dest, M2 &&A, M3 &&B) noexcept{
	dest.resize(A.rows()*B.rows(), A.cols()*B.cols());
	
	for (size_t i=0; i!=rows(A); ++i)
		for (size_t j=0; j!=cols(A); ++j){
			const typename std::decay_t<M2>::valType tempVal = A(i, j);
			for (size_t k=0; k!=rows(B); ++k)
				for (size_t l=0; l!=cols(B); ++l)
					dest(i*rows(B)+k, j*cols(B)+l) = tempVal * B(k, l);
		}
}

template<SP_MATRIX_T(M1), SP_MATRIX_T(M2), SP_MATRIX_T(M3), class Operation>
void kronApply(M1 &&dest, M2 &&A, M3 &&B, Operation &&operation) noexcept{
	dest.resize(A.rows()*B.rows(), A.cols()*B.cols());
	
	for (size_t i=0; i!=rows(A); ++i)
		for (size_t j=0; j!=cols(A); ++j){
			const typename std::decay_t<M2>::valType tempVal = A(i, j);
			for (size_t k=0; k!=rows(B); ++k)
				for (size_t l=0; l!=cols(B); ++l)
					dest(i*rows(B)+k, j*cols(B)+l) = operation(tempVal, B(k, l));
		}
}

template<auto operation, SP_MATRIX_T(M1), SP_MATRIX_T(M2), SP_MATRIX_T(B3)>
void kronApply(M1 &&dest, M2 &&A, B3 &&B) noexcept{
	dest.resize(A.rows()*B.rows(), A.cols()*B.cols());
	
	for (size_t i=0; i!=rows(A); ++i)
		for (size_t j=0; j!=cols(A); ++j){
			const typename std::decay_t<M2>::valType tempVal = A(i, j);
			for (size_t k=0; k!=rows(B); ++k)
				for (size_t l=0; l!=cols(B); ++l)
					dest(i*rows(B)+k, j*cols(B)+l) = operation(tempVal, B(k, l));
		}
}


template<SP_MATRIX_T(M)>
void swapRows(M &&dest, const size_t row1, const size_t row2) noexcept{
	for (size_t i=0; i!=dest.cols(); ++i)
		std::swap(dest(row1, i), dest(row2, i));
}

template<SP_MATRIX_T(M)>
void swapCols(M &&dest, const size_t col1, const size_t col2) noexcept{
	for (size_t i=0; i!=dest.rows(); ++i)
		std::swap(dest(i, col1), dest(i, col2));
}

template<SP_MATRIX_T(M)>
void scaleRow(M &&dest, const size_t row, const typename ::std::decay_t<M>::valType s) noexcept{
	for (size_t i=0; i!=dest.cols(); ++i) dest(row, i) *= s;
}

template<SP_MATRIX_T(M)>
void scaleColumn(M &&dest, const size_t col, const typename ::std::decay_t<M>::valType s) noexcept{
	for (size_t i=0; i!=dest.rows(); ++i) dest(i, col) *= s;
}

template<SP_MATRIX_T(M)>
void addRows(M &&dest, const size_t rowDest, const size_t rowSrc, const typename ::std::decay_t<M>::valType s = (typename ::std::decay_t<M>::valType)1) noexcept{
	for (size_t i=0; i!=dest.cols(); ++i) dest(rowDest, i) += s * dest(rowSrc, i);
}

template<SP_MATRIX_T(M)>
void addColumns(M &&dest, const size_t colDest, const size_t colSrc, const typename ::std::decay_t<M>::valType s = (typename ::std::decay_t<M>::valType)1) noexcept{
	for (size_t i=0; i!=dest.rows(); ++i) dest(i, colDest) += s * dest(i, colSrc);
}


template<SP_MATRIX_T(M)>
void luDecompose(M &&dest) noexcept{
	const size_t len = std::min(rows(dest), cols(dest));

	typename std::decay_t<M>::valType result = (typename std::decay_t<M>::valType)1;
	typename std::decay_t<M>::valType factor1, factor2;
	for (size_t i=0; i!=len; ++i){
		{
			size_t j = i;
			for (size_t k=i+1; k!=rows(dest); ++k)	// find row with max value
				j = abs(dest(k, i))>abs(dest(j, i)) ? k : j;
			if (j != i){
				for (size_t k=0; k!=cols(dest); ++k)	// exchange top row with row with max value
					std::swap(dest(i, k), dest(j, k));
				result = -result;
			}
		}
		factor1 = dest(i, i);
		result *= factor1;
		for (size_t j=i+1; j!=rows(dest); ++j){
			factor2 = dest(j, i) / factor1;
			dest(j, i) = factor2;
			for (size_t k=i+1; k!=cols(dest); ++k)
				dest(j, k) -= dest(i, k) * factor2;
		}
	}
}

template<SP_MATRIX_T(M), class Cont>
void lupDecompose(M &&dest, Cont &permuts) noexcept{
	permuts.resize(rows(dest));
	for (size_t i=0; i!=rows(dest); ++i) permuts[i] = i;

	const size_t len = std::min(rows(dest), cols(dest));

	typename std::decay_t<M>::valType factor1, factor2;
	if constexpr (std::decay_t<M>::rowMajor){
		for (size_t i=0; i!=len; ++i){
			{
				size_t j = i;
				for (size_t k=i+1; k!=rows(dest); ++k)	// find row with max value
					j = abs(dest(k, i))>abs(dest(j, i)) ? k : j;
				if (j != i){
					for (size_t k=0; k!=cols(dest); ++k)	// exchange top row with row with max value
						std::swap(dest(i, k), dest(j, k));
					std::swap(permuts[i], permuts[j]);
				}
			}
			factor1 = dest(i, i);
			for (size_t j=i+1; j!=rows(dest); ++j){
				factor2 = dest(j, i) / factor1;
				dest(j, i) = factor2;
				for (size_t k=i+1; k!=cols(dest); ++k)
					dest(j, k) -= dest(i, k) * factor2;
			}
		}
	} else{
		for (size_t i=0; i!=len; ++i){
			{
				size_t j = i;
				for (size_t k=i+1; k!=rows(dest); ++k)	// find row with max value
					j = abs(dest(k, i))>abs(dest(i, j)) ? k : j;
				if (j != i){
					for (size_t k=0; k!=cols(dest); ++k)	// exchange top row with row with max value
						std::swap(dest(k, i), dest(k, j));
					std::swap(permuts[i], permuts[j]);
				}
			}
			factor1 = dest(i, i);
			for (size_t j=i+1; j!=rows(dest); ++j){
				factor2 = dest(i, j) / factor1;
				dest(i, j) = factor2;
				for (size_t k=i+1; k!=cols(dest); ++k)
					dest(k, j) -= dest(k, j) * factor2;
			}
		}
	}
}

template<SP_MATRIX_T(M1), SP_MATRIX_T(M2)>
void extractLower(M1 &&dest, M2 &&src) noexcept{
	const size_t len = std::min(rows(src), cols(src));

	dest.resize(rows(src), len);
	src.resize(len, cols(src));

	for (size_t i=0; i!=len; ++i){
		for (size_t j=i+1; j!=rows(dest); ++j){
			dest(j, i) = src(j, i);
			src(j, i) = (typename std::decay_t<M1>::valType)0;
		}
		dest(i, i) = (typename std::decay_t<M1>::valType)1;
		for (size_t j=i+1; j!=cols(src); ++j)
			dest(i, j) = (typename std::decay_t<M1>::valType)0;
	}
}

template<SP_MATRIX_T(M1), SP_MATRIX_T(M2)>
void extractUpper(M1 &&dest, M2 &&src) noexcept{
	const size_t len = std::min(rows(src), cols(src));
	
	dest.resize(len, cols(src));
	src.resize(rows(src), len);

	for (size_t i=0; i!=len; ++i){
		for (size_t j=i+1; j!=cols(dest); ++j){
			dest(i, j) = src(i, j);
			src(i, j) = (typename std::decay_t<M1>::valType)0;
		}
		dest(i, i) = (typename std::decay_t<M1>::valType)1;
		for (size_t j=i+1; j!=rows(src); ++j)
			dest(j, i) = (typename std::decay_t<M1>::valType)0;
	}
}

template<SP_MATRIX_T(M)>
void choleskyDecompose(M &&dest) noexcept{
	SP_MATRIX_ERROR(rows(dest) != cols(dest), "only square matrix can be cholesky decomposed");
	const size_t len = rows(dest);
	if (std::decay_t<M>::rowMajor){
		for (size_t i=0; i!=len; ++i){
			for (size_t j=0; j!=i; ++j){
				typename std::decay_t<M>::valType sum = (typename std::decay_t<M>::valType)0;
				for (size_t k=0; k!=j; ++k)
					sum += dest(i, k) * dest(j, k);

				dest(i, j) = (dest(i, j) - sum) / dest(j, j);
			}
			{
				typename std::decay_t<M>::valType sum = (typename std::decay_t<M>::valType)0;
				for (size_t k=0; k!=i; ++k)
					sum += dest(i, k) * dest(i, k);
				
				dest(i, i) = sqrt(dest(i, i) - sum);
			}
		}
	} else{
		for (size_t i=0; i!=len; ++i){
			for (size_t j=0; j!=i; ++j){
				typename std::decay_t<M>::valType sum = (typename std::decay_t<M>::valType)0;
				for (size_t k=0; k!=j; ++k)
					sum += dest(k, i) * dest(k, j);

				dest(j, i) = (dest(j, i) - sum) / dest(j, j);
			}
			{
				typename std::decay_t<M>::valType sum = (typename std::decay_t<M>::valType)0;
				for (size_t k=0; k!=i; ++k)
					sum += dest(k, i) * dest(k, i);
				
				dest(i, i) = sqrt(dest(i, i) - sum);
			}
		}
	}
}

template<SP_MATRIX_T(M1), SP_MATRIX_T(M2)>
void choleskyUpdate(M1 &&dest, M2 &&A) noexcept{
	SP_MATRIX_ERROR(rows(dest)!=size(A) || cols(dest)!=size(A), "non square matrices cannot be cholesky decomposed");
	const size_t len = rows(dest);
	for (size_t i=0; i!=len; ++i){
		auto r = sqrt(dest(i, i)*dest(i, i) + A[i]*A[i]);
		auto c = r / dest(i, i);
		auto s = A[i] / dest(i, i);
		dest(i, i) = r;
		for (size_t j=i+1; j!=len; ++j){
			dest(j, i) = (dest(j, i) + s * A[j]) / c;
			A[j] = c * A[j] - s * dest(j, i);
		}
	}
}


template<SP_MATRIX_T(M), class Cont>
void permuteRows(M &&dest, const Cont &permuts) noexcept{
	static_assert(std::is_integral_v<typename Cont::value_type>, "permutation array must contain integral values");
	SP_MATRIX_ERROR(rows(dest) != std::size(permuts), "rows count of permuted matrix must be the same as size of permutation array");

	const size_t len = std::size(permuts);
	const size_t oldSize = std::size(priv__::MatrixTempStorage.data);
	priv__::MatrixTempStorage.data.expandBy((len * sizeof(typename Cont::value_type) + 7) / 8);
	typename Cont::value_type *const TempStorage = (typename Cont::value_type *)std::begin(priv__::MatrixTempStorage.data) + oldSize;
	
	std::copy_n(std::begin(dest), len, TempStorage);
	for (size_t i=0; i!=len; ++i)
		while (TempStorage[i] != i){
			const size_t swapIndex = TempStorage[i];
			std::iter_swap(std::begin(TempStorage)+i, std::begin(TempStorage)+swapIndex);

			for (size_t j=0; j!=cols(dest); ++j)
				std::swap(dest(i, swapIndex), dest(i, j));
		}

	priv__::MatrixTempStorage.data.resize(oldSize);	
}

template<SP_MATRIX_T(M), class Cont>
void permuteColumns(M &&dest, const Cont &permuts) noexcept{
	static_assert(std::is_integral_v<typename Cont::value_type>, "permutation array must contain integral values");
	SP_MATRIX_ERROR(cols(dest) != std::size(permuts), "columns count of permuted matrix must be the same as size of permutation array");

	const size_t len = std::size(permuts);
	const size_t oldSize = std::size(priv__::MatrixTempStorage.data);
	priv__::MatrixTempStorage.data.expandBy((len * sizeof(typename Cont::value_type) + 7) / 8);
	typename Cont::value_type *const TempStorage = (typename Cont::value_type *)std::begin(priv__::MatrixTempStorage.data) + oldSize;
	
	std::copy_n(std::begin(dest), len, TempStorage);
	for (size_t i=0; i!=len; ++i)
		while (TempStorage[i] != i){
			const size_t swapIndex = TempStorage[i];
			std::iter_swap(std::begin(TempStorage)+i, std::begin(TempStorage)+swapIndex);

			for (size_t j=0; j!=cols(dest); ++j)
				std::swap(dest(swapIndex, i), dest(j, i));
		}

	priv__::MatrixTempStorage.data.resize(oldSize);	
}

template<SP_MATRIX_T(M)>
void invert(M &&dest) noexcept{
	SP_MATRIX_ERROR(rows(dest) != cols(dest), "only square matrix can be inverted");
	const size_t len = rows(dest);

	const size_t oldSize = std::size(priv__::MatrixTempStorage.data);
	priv__::MatrixTempStorage.data.expandBy((len*len * sizeof(typename std::decay_t<M>::valType) + 7) / 8);
	const size_t permutsIndex = std::size(priv__::MatrixTempStorage.data);
	priv__::MatrixTempStorage.data.expandBy((len * sizeof(uint32_t) + 7) / 8);
	typename std::decay_t<M>::valType *const TempStorage = (typename std::decay_t<M>::valType *)(std::begin(priv__::MatrixTempStorage.data) + oldSize);
	uint32_t *const permuts = (uint32_t *)(std::begin(priv__::MatrixTempStorage.data) + permutsIndex);

	for (size_t i=0; i!=len; ++i) permuts[i] = i;

	if constexpr (std::decay_t<M>::rowMajor){
		typename std::decay_t<M>::valType *I = TempStorage;
		for (size_t i=0; i!=len; ++i)
			for (size_t j=0; j!=len; ++j, ++I)
				*I = dest(j, i);
	} else{
		typename std::decay_t<M>::valType *I = TempStorage;
		for (size_t i=0; i!=len; ++i)
			for (size_t j=0; j!=len; ++j, ++I)
				*I = dest(i, j);
	}

	typename std::decay_t<M>::valType factor1, factor2;
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
		factor1 = TempStorage[i*(len+1)];
		for (size_t j=i+1; j!=len; ++j){
			factor2 = TempStorage[j*len+i] / factor1;
			TempStorage[j*len+i] = factor2;
			for (size_t k=i+1; k!=len; ++k)
				TempStorage[j*len+k] -= TempStorage[i*len+k] * factor2;
		}
	}

	if constexpr (std::decay_t<M>::rowMajor){
		for (size_t i=0; i!=len; ++i){
			for (size_t j=0; j!=len; ++j){
				dest(i, j) = permuts[j]==i ? (typename std::decay_t<M>::valType)1 : (typename std::decay_t<M>::valType)0;

				for (size_t k=0; k!=j; ++k)
					dest(i, j) -= TempStorage[j*len+k] * dest(i, k);
			}

			for (size_t j=len-1; j!=(size_t)-1; --j) {
				for (size_t k = j+1; k!=len; ++k)
					dest(i, j) -= TempStorage[j*len+k] * dest(i, k);

				dest(i, j) /= TempStorage[j*(len+1)];
			}
		}
	} else{
		for (size_t i=0; i!=len; ++i){
			for (size_t j=0; j!=len; ++j){
				dest(j, i) = permuts[j]==i ? (typename std::decay_t<M>::valType)1 : (typename std::decay_t<M>::valType)0;

				for (size_t k=0; k!=j; ++k)
					dest(j, i) -= TempStorage[j*len+k] * dest(k, i);
			}

			for (size_t j=len-1; j!=(size_t)-1; --j) {
				for (size_t k = j+1; k!=len; ++k)
					dest(j, i) -= TempStorage[j*len+k] * dest(k, i);

				dest(j, i) /= TempStorage[j*(len+1)];
			}
		}
	}

	priv__::MatrixTempStorage.data.resize(oldSize);
}

template<SP_MATRIX_T(M1), SP_MATRIX_T(M2)>
void invert(M1 &&dest, M2 &&A) noexcept{
	SP_MATRIX_ERROR(rows(A) != cols(A), "only square matrix can be inverted");
	const size_t len = rows(A);

	const size_t oldSize = std::size(priv__::MatrixTempStorage.data);
	priv__::MatrixTempStorage.data.expandBy((len*len * sizeof(typename std::decay_t<M1>::valType) + 7) / 8);
	const size_t permutsIndex = std::size(priv__::MatrixTempStorage.data);
	priv__::MatrixTempStorage.data.expandBy((len * sizeof(uint32_t) + 7) / 8);
	typename std::decay_t<M1>::valType *const TempStorage = (typename std::decay_t<M1>::valType *)(std::begin(priv__::MatrixTempStorage.data) + oldSize);
	uint32_t *const permuts = (uint32_t *)(std::begin(priv__::MatrixTempStorage.data) + permutsIndex);

	for (size_t i=0; i!=len; ++i) permuts[i] = i;
	{
		typename std::decay_t<M2>::valType *I = TempStorage;
		for (size_t i=0; i!=len; ++i)
			for (size_t j=0; j!=len; ++j, ++I)
				*I = A(i, j);
	}


	// L = F*msqrt(D)
	// inv(A) = inv(tr(L)) * inv(L) = inv(tr(F*msqrt(D))) * inv(F*msqrt(D)) = tr(inv(F)) * tr(inv(msqrt(D))) * inv(msqrt(D)) * inv(F)

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
		typename std::decay_t<M2>::valType factor1 = TempStorage[i*(len+1)];
		for (size_t j=i+1; j!=len; ++j){
			typename std::decay_t<M2>::valType factor2 = TempStorage[j*len+i] / factor1;
			TempStorage[j*len+i] = factor2;
			for (size_t k=i+1; k!=len; ++k)
				TempStorage[j*len+k] -= TempStorage[i*len+k] * factor2;
		}
	}

	dest.resize(len, len);
	for (size_t i=0; i!=len; ++i){
		for (size_t j=0; j!=len; ++j){
			dest(j, i) = permuts[j]==i ? (typename std::decay_t<M1>::valType)1 : (typename std::decay_t<M1>::valType)0;

			for (size_t k=0; k!=j; ++k)
				dest(j, i) -= TempStorage[j*len+k] * dest(k, i);
		}

		for (size_t j=len-1; j!=(size_t)-1; --j) {
			typename std::decay_t<M2>::valType factor = (typename std::decay_t<M2>::valType)1 / TempStorage[j*(len+1)];
			for (size_t k = j+1; k!=len; ++k)
				dest(j, i) -= TempStorage[j*len+k] * dest(k, i);

			dest(j, i) *= factor;
		}
	}

	priv__::MatrixTempStorage.data.resize(oldSize);
}

template<SP_MATRIX_T(M1), SP_MATRIX_T(M2)>
void pinvert(M1 &&dest, M2 &&A) noexcept{
	if (rows(A) > cols(A)){
		const size_t len = cols(A);

		const size_t oldSize = std::size(priv__::MatrixTempStorage.data);
		priv__::MatrixTempStorage.data.expandBy((len*len * sizeof(typename std::decay_t<M1>::valType) + 7) / 8);
		const size_t permutsIndex = std::size(priv__::MatrixTempStorage.data);
		priv__::MatrixTempStorage.data.expandBy((len * sizeof(uint32_t) + 7) / 8);
		typename std::decay_t<M1>::valType *const TempStorage = (typename std::decay_t<M1>::valType *)(std::begin(priv__::MatrixTempStorage.data) + oldSize);
		uint32_t *const permuts = (uint32_t *)(std::begin(priv__::MatrixTempStorage.data) + permutsIndex);

		for (size_t i=0; i!=len; ++i) permuts[i] = i;
		if constexpr (std::decay_t<M2>::rowMajor){
			typename std::decay_t<M2>::valType *I = TempStorage;
			const size_t innLen = rows(A);
			for (size_t i=0; i!=len; ++i)
				for (size_t j=0; j!=i+1; ++j, ++I){
					typename std::decay_t<M2>::valType sum{};
					for (size_t k=0; k!=innLen; ++k)
						sum += A(i, k) * A(j, k);
					*I = std::move(sum);
				}
		}

		// typename std::decay_t<M>::valType factor1, factor2;
		// for (size_t i=0; i!=len; ++i){
		// 	{
		// 		size_t j = i;
		// 		for (size_t k=i+1; k!=len; ++k)	// find row with max value
		// 			j = abs(TempStorage[k*len+i])>abs(TempStorage[j*len+i]) ? k : j;
		// 		if (j != i){
		// 			for (size_t k=0; k!=len; ++k)	// exchange top row with row with max value
		// 				std::swap(TempStorage[i*len+k], TempStorage[j*len+k]);
		// 			std::swap(permuts[i], permuts[j]);
		// 		}
		// 	}
		// 	factor1 = TempStorage[i*(len+1)];
		// 	for (size_t j=i+1; j!=len; ++j){
		// 		factor2 = TempStorage[j*len+i] / factor1;
		// 		TempStorage[j*len+i] = factor2;
		// 		for (size_t k=i+1; k!=len; ++k)
		// 			TempStorage[j*len+k] -= TempStorage[i*len+k] * factor2;
		// 	}
		// }

		// if constexpr (std::decay_t<M>::rowMajor){
		// 	for (size_t i=0; i!=len; ++i){
		// 		for (size_t j=0; j!=len; ++j){
		// 			dest(i, j) = permuts[j]==i ? (typename std::decay_t<M>::valType)1 : (typename std::decay_t<M>::valType)0;

		// 			for (size_t k=0; k!=j; ++k)
		// 				dest(i, j) -= TempStorage[j*len+k] * dest(i, k);
		// 		}

		// 		for (size_t j=len-1; j!=(size_t)-1; --j) {
		// 			for (size_t k = j+1; k!=len; ++k)
		// 				dest(i, j) -= TempStorage[j*len+k] * dest(i, k);

		// 			dest(i, j) /= TempStorage[j*(len+1)];
		// 		}
		// 	}
		// }


	} else if (rows(A) == cols(A)){
		invert(dest, A);
	} else{
		invert(dest, A*tr(A));
		dest = tr(A) * cp(dest);
	}
}




template<SP_MATRIX_T(M)>
[[nodiscard]] auto trace(M &&A) noexcept{
	const size_t len = std::min(rows(A), cols(A));
	typename std::decay_t<M>::valType result = A(0, 0);
	for (size_t i=1; i!=len; ++i) result += A(i, i);
	return result;
}

template<SP_MATRIX_T(M)>
[[nodiscard]] auto determinant(M &&A) noexcept{
	SP_MATRIX_ERROR(rows(A) != cols(A), "only square matrix can have a determinant");
	const size_t len = rows(A);
	
	if constexpr (std::is_rvalue_reference_v<M>){
		typename std::decay_t<M>::valType result = typename std::decay_t<M>::valType{1};
		typename std::decay_t<M>::valType factor1, factor2;
		if constexpr (std::decay_t<M>::rowMajor){
			for (size_t i=0; i!=len-1; ++i){
				{
					size_t j = i;
					for (size_t k=i+1; k!=len; ++k)	// find row with max value
						j = abs(A(k, i))>abs(A(j, i)) ? k : j;
					if (j != i){
						for (size_t k=i; k!=len; ++k)	// exchange top row with row with max value
							std::swap(A(i, k), A(j, k));
						result = -result;
					}
				}
				factor1 = A(i, i);
				result *= factor1;
				for (size_t j=i+1; j!=len; ++j){
					factor2 = A(j, i) / factor1;
					for (size_t k=i+1; k!=len; ++k)
						A(j, k) -= A(i, k) * factor2;
				}
			}
		} else{
			for (size_t i=0; i!=len-1; ++i){
				{
					size_t j = i;
					for (size_t k=i+1; k!=len; ++k)	// find row with max value
						j = abs(A(i, k))>abs(A(i, j)) ? k : j;
					if (j != i){
						for (size_t k=i; k!=len; ++k)	// exchange top row with row with max value
							std::swap(A(k, i), A(k, j));
						result = -result;
					}
				}
				factor1 = A(i, i);
				result *= factor1;
				for (size_t j=i+1; j!=len; ++j){
					factor2 = A(j, i) / factor1;
					for (size_t k=i+1; k!=len; ++k)
						A(k, j) -= A(k, i) * factor2;
				}
			}
		}
		return result * A(len-1, len-1);
	} else{
		const size_t oldSize = std::size(priv__::MatrixTempStorage.data);
		priv__::MatrixTempStorage.data.expandBy((len*len * sizeof(typename std::decay_t<M>::valType) + 7) / 8);
		typename std::decay_t<M>::valType *const TempStorage = (typename std::decay_t<M>::valType *)std::begin(priv__::MatrixTempStorage.data) + oldSize;

		{
			typename std::decay_t<M>::valType *I = TempStorage;
			for (size_t i=0; i!=len; ++i)
				for (size_t j=0; j!=len; ++j, ++I)
					if constexpr (M::rowMajor)
						*I = A(i, j);
					else
						*I = A(j, i);
		}
		
		typename std::decay_t<M>::valType result = typename std::decay_t<M>::valType{1};
		typename std::decay_t<M>::valType factor1, factor2;
		for (size_t i=0; i!=len-1; ++i){
			{
				size_t j = i;
				for (size_t k=i+1; k!=len; ++k)	// find row with max value
					j = abs(TempStorage[k*len+i])>abs(TempStorage[j*len+i]) ? k : j;
				if (j != i){
					for (size_t k=i; k!=len; ++k)	// exchange top row with row with max value
						std::swap(TempStorage[i*len+k], TempStorage[j*len+k]);
					result = -result;
				}
			}
			factor1 = TempStorage[i*(len+1)];
			result *= factor1;
			for (size_t j=i+1; j!=len; ++j){
				factor2 = TempStorage[j*len+i] / factor1;
				for (size_t k=i+1; k!=len; ++k)
					TempStorage[j*len+k] -= TempStorage[i*len+k] * factor2;
			}
		}
		result *= TempStorage[len*len-1];
		
		priv__::MatrixTempStorage.data.resize(oldSize);
		return result;
	}
}

template<SP_MATRIX_T(M)>
[[nodiscard]] auto minor(M &&A, const size_t row, const size_t col) noexcept{
	SP_MATRIX_ERROR(rows(A) != cols(A), "only square matrix can have a minor");
	const size_t len = rows(A) - 1;
	
	if constexpr (std::is_rvalue_reference_v<M>){
		if constexpr (std::decay_t<M>::rowMajor){
			{
				for (size_t i=row; i!=len; ++i)
					for (size_t j=col; j!=len; ++j)
						A(i, j) = A(i+1, j+1);
			}
			
			typename std::decay_t<M>::valType result = (typename std::decay_t<M>::valType)1;
			typename std::decay_t<M>::valType factor1, factor2;
			for (size_t i=0; i!=len-1; ++i){
				{
					size_t j = i;
					for (size_t k=i+1; k!=len; ++k)	// find row with max value
						j = abs(A(k, i))>abs(A(j, i)) ? k : j;
					if (j != i){
						for (size_t k=i; k!=len; ++k)	// exchange top row with row with max value
							std::swap(A(i, k), A(j, k));
						result = -result;
					}
				}
				factor1 = A(i ,i);
				result *= factor1;
				for (size_t j=i+1; j!=len; ++j){
					factor2 = A(j, i) / factor1;
					for (size_t k=i+1; k!=len; ++k)
						A(j, k) -= A(i, k) * factor2;
				}
			}
			return result * A(len-1, len-1);
		} else{
			{
				for (size_t i=col; i!=len; ++i)
					for (size_t j=row; j!=len; ++j)
						A(j, i) = A(j+1, i+1);
			}
			
			typename std::decay_t<M>::valType result = (typename std::decay_t<M>::valType)1;
			typename std::decay_t<M>::valType factor1, factor2;
			for (size_t i=0; i!=len-1; ++i){
				{
					size_t j = i;
					for (size_t k=i+1; k!=len; ++k)	// find row with max value
						j = abs(A(i, k))>abs(A(i, j)) ? k : j;
					if (j != i){
						for (size_t k=i; k!=len; ++k)	// exchange top row with row with max value
							std::swap(A(i, k), A(k, j));
						result = -result;
					}
				}
				factor1 = A(i ,i);
				result *= factor1;
				for (size_t j=i+1; j!=len; ++j){
					factor2 = A(i, j) / factor1;
					for (size_t k=i+1; k!=len; ++k)
						A(k, j) -= A(i, k) * factor2;
				}
			}
			return result * A(len-1, len-1);
		}
	} else{
		const size_t oldSize = std::size(priv__::MatrixTempStorage.data);
		priv__::MatrixTempStorage.data.expandBy((len*len * sizeof(typename std::decay_t<M>::valType) + 7) / 8);
		typename std::decay_t<M>::valType *const TempStorage = (typename std::decay_t<M>::valType *)std::begin(priv__::MatrixTempStorage.data) + oldSize;

		{
			typename std::decay_t<M>::valType *I = TempStorage;
			for (size_t i=0; i!=row; ++i){	// Make submatrix
				for (size_t j=0; j!=col; ++j, ++I)
					*I = A(i, j);
				for (size_t j=col; j!=len; ++j, ++I)
					*I = A(i, j+1);
			}
			for (size_t i=row+1; i!=len+1; ++i){
				for (size_t j=0; j!=col; ++j, ++I)
					*I = A(i, j);
				for (size_t j=col; j!=len; ++j, ++I)
					*I = A(i, j+1);
			}
		}
		
		typename std::decay_t<M>::valType result = (typename std::decay_t<M>::valType)1;
		typename std::decay_t<M>::valType factor1, factor2;
		for (size_t i=0; i!=len-1; ++i){
			{
				size_t j = i;
				for (size_t k=i+1; k!=len; ++k)	// find row with max value
					j = abs(TempStorage[k*len+i])>abs(TempStorage[j*len+i]) ? k : j;
				if (j != i){
					for (size_t k=i; k!=len; ++k)	// exchange top row with row with max value
						std::swap(TempStorage[i*len+k], TempStorage[j*len+k]);
					result = -result;
				}
			}
			factor1 = TempStorage[i*(len+1)];
			result *= factor1;
			for (size_t j=i+1; j!=len; ++j){
				factor2 = TempStorage[j*len+i] / factor1;
				for (size_t k=i+1; k!=len; ++k)
					TempStorage[j*len+k] -= TempStorage[i*len+k] * factor2;
			}
		}
		result *= TempStorage[len*len-1];
		
		priv__::MatrixTempStorage.data.resize(oldSize);
		return result;
	}
}

template<SP_MATRIX_T(M)>
[[nodiscard]] auto cofactor(M &&A, const size_t row, const size_t col) noexcept{
	SP_MATRIX_ERROR(rows(A) != cols(A), "only square matrix can have a cofactor");
	const auto result = minor(std::forward<M>(A), row, col);
	return (row + col)&1 ? -result : result;
}






template<SP_MATRIX_T(M)>
[[nodiscard]] bool isLowerTriangular(M &&A) noexcept{
	for (size_t i=0; i!=A.rows(); ++i)
		for (size_t j=i+1; j!=A.cols(); ++j)
			if (A(i, j) != (typename std::decay_t<M>::valType)0) return false;
	return true;
}

template<SP_MATRIX_T(M)>
[[nodiscard]] bool isUpperTriangular(M &&A) noexcept{
	for (size_t i=0; i!=A.cols(); ++i)
		for (size_t j=i; j!=A.rows(); ++j)
			if (A(j, i) != (typename std::decay_t<M>::valType)0) return false;
	return true;
}

template<SP_MATRIX_T(M)>
[[nodiscard]] bool isSymmetric(M &&A) noexcept{
	for (size_t i=0; i!=A.rows(); ++i)
		for (size_t j=i+1; j!=A.rows(); ++j)
			if (A(i, j) != A(j, i)) return false;
	return true;
}

template<SP_MATRIX_T(M)>
[[nodiscard]] bool isDiagonal(M &&A) noexcept{
	for (size_t i=0; i!=A.rows(); ++i)
		for (size_t j=i+1; j!=A.rows(); ++j)
			if (A(i, j)!=(typename std::decay_t<M>::valType)0 && A(j, i)!=(typename std::decay_t<M>::valType)0) return false;
	return true;
}


} namespace{


template<SP_MATRIX_T(M1), SP_MATRIX_T(M2)>
[[nodiscard]] bool operator ==(M1 &&lhs, M2 &&rhs) noexcept{
	if (rows(lhs)!=rows(rhs) || cols(lhs)!=cols(rhs)) return false;
	if constexpr (M1::rowMajor || M2::rowMajor){
		for (size_t i=0; i!=rows(lhs); ++i)
			for (size_t j=0; j!=cols(lhs); ++j)
				if (lhs(i, j) != rhs(i, j)) return false;
	} else{
		for (size_t i=0; i!=cols(lhs); ++i)
			for (size_t j=0; j!=rows(lhs); ++j)
				if (lhs(j, i) != rhs(j, i)) return false;
	}
	return true;
}

template<SP_MATRIX_T(M1), SP_MATRIX_T(M2)> bool operator !=(M1 &&lhs, const M2 &&rhs) noexcept{ return !(lhs == rhs); }



} // END OF OPEARTORS























namespace sp{



template<SP_VECTOR_T(V)>
constexpr auto rank(V &&A) noexcept{ return 1; }

template<SP_VECTOR_T(V)>
constexpr auto trace(V &&A) noexcept{ return size(A) ? A[0] : (typename std::decay_t<V>::valType)0; }




// template<class Cont>
// void permute(Cont &dest) noexcept{
// 	static_assert(std::is_integral_v<typename Cont::value_type>, "permutation array must contain integral values");
// 	for (size_t i=0; i!=std::size(dest)-1; ++i)
// 		while (i != dest[i])
// 			std::swap(dest[i], dest[dest[i]]);
// }

template<class Cont>
void invertPermuts(Cont &dest) noexcept{
	static_assert(std::is_integral_v<typename Cont::value_type>, "permutation array must contain integral values");

	const size_t len = std::size(dest);
	const size_t oldSize = std::size(priv__::MatrixTempStorage.data);
	priv__::MatrixTempStorage.data.expandBy((len*len * sizeof(typename Cont::value_type) + 7) / 8);
	typename Cont::value_type *const TempStorage = (typename Cont::value_type *)std::begin(priv__::MatrixTempStorage.data) + oldSize;
	
	std::copy_n(std::begin(dest), len, TempStorage);
	for (size_t i=0; i!=len; ++i)
		dest[i] = TempStorage[(size_t)TempStorage[i]];

	priv__::MatrixTempStorage.data.resize(oldSize);
}

template<class Cont1, class Cont2>
void invertPermuts(Cont1 &dest, const Cont2 &src) noexcept{
	static_assert(std::is_integral_v<typename Cont1::value_type> && std::is_integral_v<typename Cont2::value_type>, "permutation array must contain integral values");

	dest.resize(std::size(src));
	for (size_t i=0; i!=std::size(dest); ++i)
		dest[i] = src[(size_t)src[i]];
}










} namespace{
using namespace sp;

template<SP_VECTOR_T(V1), SP_VECTOR_T(V2)>
auto operator *(V1 &&lhs, V2 &&rhs) noexcept{
	SP_MATRIX_ERROR(size(lhs) != size(rhs), "operands of inner product cannot have different lengths");
	typename std::decay_t<V1>::valType result = (typename std::decay_t<V1>::valType)0;
	for (size_t i=0; i!=size(lhs); ++i)
		result += lhs[i] * rhs[i];
	return result;
}

template<SP_VECTOR_T(V1), SP_VECTOR_T(V2)>
bool operator ==(V1 &&lhs, V2 &&rhs) noexcept{
	if (size(lhs) != size(rhs)) return false;
	for (size_t i=0, j; i!=size(lhs); ++i)
		if (lhs[i] != rhs[i]) return false;
	return true;
}

template<SP_VECTOR_T(V1), SP_VECTOR_T(V2)> bool operator !=(V1 &&lhs, V2 &&rhs) noexcept{ return !(lhs == rhs); }


} // END OF NAMESPACE ///////////////////////////////////////////////////////////////////