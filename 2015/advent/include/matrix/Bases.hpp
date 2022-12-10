#pragma once
#include "SPL/Utils.hpp"
#include "SPL/Arrays.hpp"

#ifdef SP_MATRIX_DEBUG
	#define SP_MATRIX_ERROR(cond, msg) if (cond){ puts(msg); abort(); }
#else
	#define SP_MATRIX_ERROR(cond, msg)
#endif

namespace sp::priv__{


enum class StupidMatrixFlagType{};
enum class StupidVectorFlagType{};

#define SP_MATRIX_T(M) class M, ::sp::priv__::StupidMatrixFlagType = ::std::decay_t<M>::MatrixFlag
#define SP_VECTOR_T(V) class V, ::sp::priv__::StupidVectorFlagType = ::std::decay_t<V>::VectorFlag


template<class T> struct CRemRRefStruct{ typedef T type; };
template<class T> struct CRemRRefStruct<T &&>{ typedef T type; };
template<class T> struct CRemRRefStruct<T &>{ typedef const T &type; };
template<class T> using CRemRRef = typename CRemRRefStruct<T>::type;

template<class T> struct RemRRefStruct{ typedef T type; };
template<class T> struct RemRRefStruct<T &&>{ typedef T type; };
template<class T> using RemRRef = typename RemRRefStruct<T>::type;




struct Fsdlfjiosdjfpowejhefjsdf{
	union{
		sp::DArray<uint64_t> data;
	};
	size_t stackSize = 0;

	Fsdlfjiosdjfpowejhefjsdf() : data() {}
	~Fsdlfjiosdjfpowejhefjsdf(){}
} MatrixTempStorage;





template<class T, bool rowMaj, size_t rowsNumber, size_t colsNumber>
struct MatrixStatic{
	typedef T valType;
	static constexpr StupidMatrixFlagType MatrixFlag{};
	constexpr static bool rowMajor = rowMaj;
	constexpr static bool undefMajor = false;
	constexpr static bool usesBuffer = false;

	MatrixStatic() noexcept{}
	MatrixStatic(const MatrixStatic &) noexcept = default;
	MatrixStatic &operator =(const MatrixStatic &) noexcept = default;

	[[nodiscard]] constexpr size_t rows() const noexcept{ return rowsNumber; }
	[[nodiscard]] constexpr size_t cols() const noexcept{ return colsNumber; }
	[[nodiscard]] constexpr size_t size() const noexcept{ return rowsNumber * colsNumber; }
	[[nodiscard]] constexpr size_t capacity() const noexcept{ return 0; }

	constexpr void resize(const size_t r, const size_t c) noexcept{ SP_MATRIX_ERROR(r!=rowsNumber || c!=colsNumber, "static matrix cannot be resized"); }

	constexpr valType &operator ()(const size_t r, const size_t c) noexcept{
		SP_MATRIX_ERROR(r>=rowsNumber || c>=colsNumber, "out of bounds matrix indecies");
		if constexpr (rowMajor)
			return data[r*colsNumber + c];
		else
			return data[r + c*rowsNumber];
	}
	[[nodiscard]] constexpr const valType &operator ()(const size_t r, const size_t c) const noexcept{
		SP_MATRIX_ERROR(r>=rowsNumber || c>=colsNumber, "out of bounds matrix indecies");
		if constexpr (rowMajor)
			return data[r*colsNumber + c];
		else
			return data[r + c*rowsNumber];
	}

	T data[rowsNumber * colsNumber];
};


template<class T, bool rowMaj, size_t cap>
struct MatrixHybrid{
	typedef T valType;
	static constexpr StupidMatrixFlagType MatrixFlag{};
	constexpr static bool rowMajor = rowMaj;
	constexpr static bool undefMajor = false;
	constexpr static bool usesBuffer = false;

	MatrixHybrid() noexcept{ rowsNumber = 0; colsNumber = 0; }
	MatrixHybrid(const MatrixHybrid &) noexcept = default;
	MatrixHybrid &operator =(const MatrixHybrid &) noexcept = default;

	[[nodiscard]] constexpr size_t rows() const noexcept{ return rowsNumber; }
	[[nodiscard]] constexpr size_t cols() const noexcept{ return colsNumber; }
	[[nodiscard]] constexpr size_t size() const noexcept{ return rowsNumber * colsNumber; }
	[[nodiscard]] constexpr size_t capacity() const noexcept{ return cap; }

	constexpr void resize(const size_t r, const size_t c) noexcept{
		SP_MATRIX_ERROR(r*c>cap, "requested size exceeds the capacity");
		rowsNumber = r; colsNumber = c;
	}

	constexpr valType &operator ()(const size_t r, const size_t c) noexcept{
		SP_MATRIX_ERROR(r>=rowsNumber || c>=colsNumber, "out of bounds matrix indecies");
		if constexpr (rowMajor)
			return data[r*(size_t)colsNumber + c];
		else
			return data[r + c*(size_t)rowsNumber];
	}
	[[nodiscard]] constexpr const valType &operator ()(const size_t r, const size_t c) const noexcept{
		SP_MATRIX_ERROR(r>=rowsNumber || c>=colsNumber, "out of bounds matrix indecies");
		if constexpr (rowMajor)
			return data[r*(size_t)colsNumber + c];
		else
			return data[r + c*(size_t)rowsNumber];
	}



	union{
		T data[cap];
	};
	uint32_t rowsNumber, colsNumber;
};


template<class T, bool rowMaj>
struct MatrixPoolAlloc{
	typedef T valType;
	static constexpr StupidMatrixFlagType MatrixFlag{};
	constexpr static bool rowMajor = rowMaj;
	constexpr static bool undefMajor = false;
	constexpr static bool usesBuffer = false;

	MatrixPoolAlloc(const size_t capacity) noexcept{
		dataIndex = MatrixTempStorage.stackSize;
		rowsNumber = 0;
		colsNumber = 0;
		cap = capacity;
		const size_t expansionSize = (capacity * sizeof(T) + 7) / 8;
		MatrixTempStorage.data.expandBy(expansionSize);
		MatrixTempStorage.stackSize += expansionSize;
	}
	MatrixPoolAlloc(MatrixPoolAlloc &&A) noexcept{
		dataIndex = A.dataIndex;
		rowsNumber = A.cols();
		colsNumber = A.rows();
		cap = A.capacity();
		A.cap = 0;
	}
	~MatrixPoolAlloc() noexcept{
		const size_t expansionSize = (cap * sizeof(T) + 7) / 8;
		MatrixTempStorage.data.shrinkBy(expansionSize);
		MatrixTempStorage.stackSize -= expansionSize;
	}
	MatrixPoolAlloc &operator =(const MatrixPoolAlloc &rhs) noexcept{
		this->resize(rhs.rows(), rhs.cols());
		std::copy_n((const T *)&MatrixTempStorage.data[rhs.dataIndex], this->size(), (T *)&MatrixTempStorage.data[dataIndex]);
		return *this;
	}

	[[nodiscard]] constexpr size_t rows() const noexcept{ return rowsNumber; }
	[[nodiscard]] constexpr size_t cols() const noexcept{ return colsNumber; }
	[[nodiscard]] constexpr size_t size() const noexcept{ return rowsNumber * colsNumber; }
	[[nodiscard]] constexpr size_t capacity() const noexcept{ return cap; }

	constexpr void resize(const size_t r, const size_t c) noexcept{
		SP_MATRIX_ERROR(r*c>cap, "requested size exceeds the capacity");
		rowsNumber = r; colsNumber = c;
	}

	constexpr valType &operator ()(const size_t r, const size_t c) noexcept{
		SP_MATRIX_ERROR(r>=rowsNumber || c>=colsNumber, "out of bounds matrix indecies");
		if constexpr (rowMajor)
			return *((T *)&MatrixTempStorage.data[dataIndex] + r*(size_t)colsNumber + c);
		else
			return *((T *)&MatrixTempStorage.data[dataIndex] + r + c*(size_t)rowsNumber);
	}
	[[nodiscard]] constexpr const valType &operator ()(const size_t r, const size_t c) const noexcept{
		SP_MATRIX_ERROR(r>=rowsNumber || c>=colsNumber, "out of bounds matrix indecies");
		if constexpr (rowMajor)
			return *((const T *)&MatrixTempStorage.data[dataIndex] + r*(size_t)colsNumber + c);
		else
			return *((const T *)&MatrixTempStorage.data[dataIndex] + r + c*(size_t)rowsNumber);
	}


	uint32_t dataIndex;
	uint32_t rowsNumber, colsNumber;
	uint32_t cap;
};



























template<class T, size_t len>
struct VectorStatic{
	typedef T valType;
	static constexpr StupidVectorFlagType VectorFlag{};
	constexpr static bool usesBuffer = false;

	VectorStatic() noexcept{}
	VectorStatic(const VectorStatic &) noexcept = default;
	VectorStatic &operator =(const VectorStatic &) noexcept = default;

	[[nodiscard]] constexpr size_t size() const noexcept{ return len; }
	[[nodiscard]] constexpr size_t capacity() const noexcept{ return 0; }

	constexpr void resize(const size_t n) noexcept{ SP_MATRIX_ERROR(n != len, "static vector cannot be resized"); }

	constexpr valType &operator [](const size_t i) noexcept{
		SP_MATRIX_ERROR(i >= len, "out of bounds vector index");
		return data[i];
	}
	[[nodiscard]] constexpr const valType &operator [](const size_t i) const noexcept{
		SP_MATRIX_ERROR(i >= len, "out of bounds vector index");
		return data[i];
	}


	T data[len];
};


template<class T, size_t cap>
struct VectorHybrid{
	typedef T valType;
	static constexpr StupidVectorFlagType VectorFlag{};
	constexpr static bool usesBuffer = false;

	VectorHybrid() noexcept{ len = 0; }
	VectorHybrid(const VectorHybrid &) noexcept = default;
	VectorHybrid &operator =(const VectorHybrid &) noexcept = default;

	[[nodiscard]] constexpr size_t size() const noexcept{ return len; }
	[[nodiscard]] constexpr size_t capacity() const noexcept{ return cap; }

	constexpr void resize(const size_t n) noexcept{ SP_MATRIX_ERROR(n>cap, "requested size exceeds the capacity"); len = n; }

	constexpr valType &operator [](const size_t i) noexcept{
		SP_MATRIX_ERROR(i >= len, "out of bounds vector index");
		return data[i];
	}
	[[nodiscard]] constexpr const valType &operator [](const size_t i) const noexcept{
		SP_MATRIX_ERROR(i >= len, "out of bounds vector index");
		return data[i];
	}


	union{
		T data[cap];
	};
	uint32_t len;
};



} // END OF NAMESPACE ///////////////////////////////////////////////////////////////////////////////////////