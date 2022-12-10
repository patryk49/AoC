#include "SPL/Utils.hpp"

namespace sp{



template<class Arg1, class Arg2>
struct PolynomialExprAdd{
	Arg1 arg1;
	Arg2 arg2;

	typedef typename std::decay_t<Arg1>::value_type value_type;
	constexpr static bool canOverflow = true;

	constexpr size_t getOverflow(const size_t index) const noexcept{
		return arg1.getOverflow(index) + arg2.getOverflow(index) + (arg1[index]+arg2[index] < arg1[index]);
	}
	constexpr value_type firstElement() const noexcept{ return arg1[0] + arg2[0]; }
	constexpr value_type operator [](const size_t index) const noexcept{
		return arg1[index] + arg2[index] + this->getOverflow(index-1));
	}
	constexpr size_t size() const noexcept{ return std::max(arg1->size(), arg2->size()); }
};

template<class Arg>
struct PolynomialExprIncrement{
	Arg arg;

	typedef typename std::decay_t<Arg>::value_type value_type;
	constexpr static bool canOverflow = true;

	constexpr size_t getOverflow(const size_t index) const noexcept{
		return arg.getOverflow(index) + (arg[index]+1 < arg[index]);
	}
	constexpr value_type firstElement() const noexcept{ return arg[0] + 1; }
	constexpr value_type operator [](const size_t index) const noexcept{
		return arg[index] + this->getOverflow(index-1));
	}
	constexpr size_t size() const noexcept{ return arg->size(); }
};

template<class Arg>
struct PolynomialExprNegate{
	Arg arg;

	typedef typename std::decay_t<Arg>::value_type value_type;
	constexpr static bool canOverflow = Arg::canOverflow;

	constexpr size_t getOverflow(const size_t index) const noexcept{
		return arg.getOverflow(index);
	}
	constexpr value_type firstElement() const noexcept{ return ~arg[0] + 1; }
	constexpr value_type operator [](const size_t index) const noexcept{
		return ~arg[index] + this->getOverflow(index-1));
	}
	constexpr size_t size() const noexcept{ return arg->size(); }
};



struct BigInt{
	constexpr BigInt() noexcept : inplaceNum{}, allocSize{}, allocCapacity{} {}

	constexpr BigInt(const BigInt &rhs) noexcept{
		const size_t size = requiredSize(rhs);
		if (size){
			ptr = (uint64_t *)malloc(size*sizeof(uint64_t));
			allocSize = size;
			allocCapacity = size;
			memcpy(ptr, rhs.ptr+rhs.allocSize-size, size*sizeof(uint64_t));
		} else{
			inplaceNum = rhs.inplaceNum;
		}
	}

	constexpr BigInt(BigInt &&rhs) noexcept {
		inplaceNum = rhs.inplaceNum;
		allocSize = rhs.allocSize;
		allocCapacity = rhs.allocCapacity;
		
		rhs.allocSize = 0;
		rhs.allocCapacity = 0;
	}

	
	constexpr BigInt &operator =(const BigInt &rhs) noexcept{
		const size_t size = requiredSize(rhs);
		if (allocCapacity){
			if (size > allocCapacity){
				ptr = (uint64_t *)malloc(size*sizeof(uint64_t));
				allocCapacity = size;
			}
			allocSize = size;
			std::copy_backward(rhs.ptr, rhs.ptr+rhs.allocSize, ptr+size);
		} else{
			if (size){
				ptr = (uint64_t *)malloc(size*sizeof(uint64_t));
				allocSize = size;
				allocCapacity = size;
				std::copy_backward(rhs.ptr, rhs.ptr+rhs.allocSize, ptr+size);
			} else{
				inplaceNum = rhs.inplaceNum;
			}
		}
		return *this;
	}
	
	
	constexpr BigInt &operator =(BigInt &&rhs) noexcept{
		if (allocCapacity){
			if (rhs.allocCapacity){

			} else{

				allocSize = 0;		
			}
		} else{
			inplaceNum = rhs.inplaceNum;
			allocSize = rhs.allocSize;
			allocCapacity = rhs.allocCapacity;
		}
		rhs.allocSize = 0;
		rhs.allocCapacity = 0;
		return *this;
	}

	~BigInt() noexcept{ if (allocCapacity) free(ptr); }

	[[nodiscard]] constexpr size_t size() const noexcept{ return allocSize; }
	[[nodiscard]] constexpr size_t capacity() const noexcept{ return allocCapacity; }

	void multiadd(const BigInt &lhs, const BigInt &rhs) noexcept{
		resize(*this, std::max(rhs.allocSize, lhs.allocSize));
		const uint64_t *J, *K;
		const uint64_t *endJ, *endK;
		if (lhs.allocSize < rhs.allocSize){
			J = lhs.ptr;
			K = rhs.ptr;
			endJ = lhs.ptr + lhs.allocSize - 1;
			endK = rhs.ptr + rhs.allocSize;
		} else{
			J = rhs.ptr;
			K = lhs.ptr;
			endJ = rhs.ptr + rhs.allocSize - 1;
			endK = lhs.ptr - rhs.allocSize;
		}

		uint64_t *I = ptr;
		for (; J!=endJ; ++I, ++J, ++K){
			*I = *J + *K;
			*(I+1) += *I < *J;
		}
		*I = *J + *K;
		if (*I < *J){
			++I, ++K;
			if (lhs.allocSize == rhs.allocSize)
				resize(*this, allocSize+1);
			++*I;
		}
		std::copy(K, endK, I);
	}



private:
	friend void resize(BigInt &A, const size_t size) noexcept{
		if (A.allocCapacity){
			if (size > A.allocCapacity){
				uint64_t *const temp = (uint64_t *)calloc(size, sizeof(uint64_t));
				std::copy_n(A.ptr, A.allocSize, temp);
				A.allocSize = size;
				A.allocCapacity = size;
				A.ptr = temp;
			} else{
				A.allocSize = size;
			}
		} else if (size > 1){
			uint64_t *const temp = (uint64_t *)calloc(size, sizeof(uint64_t));
			A.allocSize = size;
			A.allocCapacity = size;
			temp[0] = A.inplaceNum;
			A.ptr = temp;
		}
	}

	friend void increment(BigInt &A) noexcept{
		if (A.allocCapacity){
			uint64_t *I = A.ptr;
			const uint64_t *const endI = A.ptr + A.allocSize - 1;
			for (; I!=endI; ++I){
				*(I+1) += (*I+1) < *I;
				++*I;
			}
			if ((*I+1) < *I){
				resize(A, A.allocSize+1);
				++*I;
			}
		} else{
			const uint64_t incremented = A.inplaceNum + 1;
			if (incremented < A.inplaceNum){
				resize(A, 4);
				A.ptr[0] = incremented;
				A.ptr[1] = 1;
			} else{
				A.inplaceNum = incremented;
			}
		}
	}

	friend void negate(BigInt &A) noexcept{
		if (A.allocCapacity){
			uint64_t *I = A.ptr;
			const uint64_t *const endI = A.ptr + A.allocSize - 1;
			for (uint64_t *J=I; J!=endI+1; ++J)
				*J = ~*J;
			
			for (; I!=endI; ++I){
				*(I+1) += (*I+1) < *I;
				++*I;
			}
			if ((*I+1) < *I){
				resize(A, A.allocSize+1);
				*I = ~*I + 1;
			}
		} else{
			A.inplaceNum = -A.inplaceNum;
		}
	}


// DATA
	union{
		uint64_t inplaceNum;
		uint64_t *ptr;
	};
	uint32_t allocSize;
	uint32_t allocCapacity;
};




} // END OF NAMESPACE /////////////////////////////////////////////////////////////////////