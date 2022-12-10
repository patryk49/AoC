#include "SPL/Utils.hpp"

namespace sp{



template<size_t BlockNum>
struct BigInt{
	static_assert(BlockNum > 1, "this makes no sense");
	struct Splitter{ uint32_t upper; uint32_t lower; };

	template<size_t B>
	BigInt &operator +=(const BigInt<B> &rhs) noexcept{
		uint64_t *I = data + BlockNum;
		const uint64_t *J = rhs.data + B;
		if constexpr (BlockNum <= B){
			while (--I != data){
				*I += *--J;
				*(I-1) += *I < *J;
			}
			*I += *--J;
		} else{
			while (J != rhs.data){
				*--I += *--J;
				*(I-1) += *I < *J;
			}
		}
	}

	template<size_t B>
	BigInt &operator -=(const BigInt<B> &rhs) noexcept{
		uint64_t *I = data + BlockNum;
		const uint64_t *J = rhs.data + B;
		if constexpr (BlockNum <= B){
			while (--I != data){
				*I -= *--J;
				*(I-1) -= *I < *J;
			}
			*I -= *--J;
		} else{
			while (J != rhs.data){
				*--I -= *--J;
				*(I-1) -= *I < *J;
			}
		}
	}

	template<size_t B>
	BigInt &operator *=(const BigInt<B> &rhs) noexcept{
		uint64_t *I = data + BlockNum;
		// const uint64_t *J = rhs.data + B;
		// if constexpr (BlockNum <= B){
		// 	while (--I != data){
		// 		I->
		// 		*(I-1) -= *I < *J;
		// 	}
		// 	*I -= *--J;
		// } else{
		// 	while (J != rhs.data){
		// 		*--I -= *--J;
		// 		*(I-1) -= *I < *J;
		// 	}
		// }
	}

	static constexpr size_t blockNumber = BlockNum;

	union{
		uint64_t data[BlockNum];
		Splitter splitData[BlockNum];
	};
};




} // END OF NAMESPACE /////////////////////////////////////////////////////////////////////