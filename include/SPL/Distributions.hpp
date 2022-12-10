#pragma once

#include "Utils.hpp"

namespace sp{ // BEGINING OF NAMESPACE ///////////////////////////////////////////////////////////

template<class T> struct UnifDistr;

struct UnifDistr<uint32_t>{
	template<class R>
	SP_CI uint32_t operator ()(R &rng) const noexcept{
		uint32_t range = max - min + 1;
		uint32_t mask = roundUpPow2(range) - 1;
		for (;;){
			uint32_t n = (uint32_t)rng() & mask;
			if (n < range) return min + n;
		}
	}

	uint32_t min;
	uint32_t max;
};
struct UnifDistr<int32_t>{
	template<class R>
	SP_CI int32_t operator ()(R &rng) const noexcept{
		int32_t range = max - min + 1;
		int32_t mask = roundUpPow2(range) - 1;
		for (;;){
			int32_t n = (int32_t)rng() & mask;
			if (n < range) return min + n;
		}
	}

	int32_t min;
	int32_t max;
};

struct UnifDistr<float>{
	template<class R>
	SP_CI int32_t operator ()(R &rng) const noexcept{
		return min + (float)rng()/(float)(maxVal(rng)-minVal(rng)) * (max-min);
	}

	float min;
	float max;
};



} // END OF NAMESPACE	///////////////////////////////////////////////////////////////////
