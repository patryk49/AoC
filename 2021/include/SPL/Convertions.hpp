#include "SPL/Utils.hpp"


namespace sp{


template<size_t N = 10>
bool tochars(char *first, const size_t size, uint64_t x, const char pads = ' ') noexcept{
	char *last = first + size;
	--first;
	size_t n = 1;
	while (x){
		[[unlikely]] if (--last == first) return false;
		const uint32_t next = x / N;
		if constexpr (N <= 10)
			*last = '0' + (x - N*next);
		else
			*last = "0123456789abcdef"[x - N*next];
		x = next;
	}
	while (--last != first) *last = pads;
	return true;
}

template<size_t N = 10>
bool tochars(char *first, const size_t size, int64_t x, const char pads = ' ') noexcept{
	char *last = first + size;
	--first;
	const bool isNegative = x < 0;
	if (isNegative) x = -x;

	size_t n = 1;
	while (x){
		[[unlikely]] if (--last == first) return false;
		const uint32_t next = x / N;
		if constexpr (N <= 10)
			*last = '0' + (x - N*next);
		else
			*last = "0123456789abcdef"[x - N*next];
		x = next;
	}
	if (isNegative){
		[[unlikely]] if (--last == first) return false;
		*last = '-';
	}

	while (--last != first) *last = pads;
	return true;
}



template<size_t N = 10>
bool tochars(char *first, const size_t size, uint32_t x, const char pads = ' ') noexcept{
	return tochars<N>(first, size, (uint64_t)x, pads);
}	
template<size_t N = 10>
bool tochars(char *first, const size_t size, uint16_t x, const char pads = ' ') noexcept{
	return tochars<N>(first, size, (uint64_t)x, pads);
}	
template<size_t N = 10>
bool tochars(char *first, const size_t size, uint8_t x, const char pads = ' ') noexcept{
	return tochars<N>(first, size, (uint64_t)x, pads);
}	



template<size_t N = 10>
bool tochars(char *first, const size_t size, int32_t x, const char pads = ' ') noexcept{
	return tochars<N>(first, size, (int64_t)x, pads);
}	
template<size_t N = 10>
bool tochars(char *first, const size_t size, int16_t x, const char pads = ' ') noexcept{
	return tochars<N>(first, size, (int64_t)x, pads);
}	
template<size_t N = 10>
bool tochars(char *first, const size_t size, int8_t x, const char pads = ' ') noexcept{
	return tochars<N>(first, size, (int64_t)x, pads);
}



} // END OF NAMESPACE ///////////////////////////////////////////////////
