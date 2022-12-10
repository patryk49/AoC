#pragma once

#include "SPL/Utils.hpp"
#include <numeric>

namespace sp{ // BEGINING OF NAMESPACE //////////////////////////////////////////////////////////////////

template<class It>
void insertionSort(It begin, It end);

template<class It, class Compare>
void insertionSort(It begin, It end, Compare compare);

template<class It>
void heapSort(It begin, It end);

template<class It, class Compare>
void heapSort(It begin, It end, Compare compare);


template<class T>
void selectionSort(T *begin, T *end);

template<class T>
void bubbleSort(T *begin, T *end);

template<class T>
void gnomeSort(T *begin, T *end);

template<class T>
void bogoSort(T *begin, T *end);

template<class T>
void quickSort(T *begin, T *end);

template<class T>
void quickInsertionSort(T *begin, T *end);

template<class T>
inline void mergeSort(T *begin, T *end);

template<class T>
void coctailShakerSort(T *begin, T *end);

template<class T>
void shellSort(T *begin, T *end);

template<class T>
void combSort(T *begin, T *end);

template<class T>
void slowSort(T *begin, T *end);





template<class It>
void insertionSort(It begin, It end){
	typedef typename std::iterator_traits<It>::value_type Type;
	/* MAKE SURE THE FIRST ELEMENT IS AN EXTREMUM */{	
		It min = begin;
		for (It I=begin+1; I<end; ++I)
			min = *I<*min ? I : min;
		std::iter_swap(begin, min);
	}	
	for (It I=begin+2; I<end; ++I){
		Type temp = std::move(*I);
		It J = I;
		while (temp < *--J) *(J+1) = std::move(*J);
		*(J+1) = std::move(temp);
	}
}

template<class It, class Compare>
void insertionSort(It begin, It end, Compare compare){
	typedef typename std::iterator_traits<It>::value_type Type;
	if (begin == end)
		return;
	if constexpr (compare(Type{}, Type{}) ||
		!std::is_same<typename std::iterator_traits<It>::iterator_category, typename std::random_access_iterator_tag>()
	){
		for (It I=begin+1; I!=end; ++I){
			Type temp = std::move(*I);
			if (compare(temp, *begin)){
				for (It J=I, K=I-1; J!=begin; --J, --K)
					*J = std::move(*K);
				*begin = std::move(temp);
			} else{
				It K = I;
				for (It J=I-1; compare(temp, *J); --J){
					*K = std::move(*J);
					K = J;
				}
				*K = std::move(temp);
			}
		}
	} else{
		makeHeap(begin, end, compare);
		for (It I=begin+2; I<end; ++I){
			Type temp = std::move(*I);
			It K = I;
			for (It J=I-1; compare(temp, *J); --J){
				*K = std::move(*J);
				K = J;
			}
			*K = std::move(temp);
		}
	}
}

template<class It>
inline void heapSort(It begin, It end){
	sp::makeHeap(begin, end);

	for (It I=(end)-1; I!=begin; --I){
		std::iter_swap(begin, I);
		sp::repairHeap(begin, I);
	}
}

template<class It, class Compare>
void heapSort(It begin, It end, Compare compare){
	sp::makeHeap(begin, end, compare);

	for (It I=(end)-1; I!=begin; --I){
		std::iter_swap(begin, I);
		sp::repairHeap(begin, I, compare);
	}
}

template<class T>
void selectionSort(T *begin, T *end){
	T *max;
	for (T *I=begin, *J; I!=end; ++I){
		for (J=I+1, max=I; J!=end; ++J)
			max = *max<*J ? max : J;
		std::iter_swap(I, max);
	}
}

template<class T>
void bubbleSort(T *begin, T *end){
	for (T *I; begin!=end; --end)
		for (I=begin+1; I<end; ++I)
			if (*I < *(I-1))
				std::iter_swap(I, I-1);
}

template<class T>
void gnomeSort(T *begin, T *end){
	for (T *I=begin+1, *J; I!=end; ++I)
		for(J=I; J!=begin; --J){
			if (*(J-1) < *J)
				break;
			std::iter_swap(J, J-1);
		}
}

template<class T>
void bogoSort(T *begin, T *end){
	while (true){
		std::random_shuffle(begin, end);
		for (T *I=begin+1; I!=end; ++I)
			if (*I < *(I-1))
				return;
	}
}

template<class T>
void quickSort(T *begin, T *end){
	T *lastElPtr = (T *)(end) - 1;
	if (begin >= lastElPtr) return;
	
	T *border = begin;
	for (T *I=begin; I<lastElPtr; ++I){
		if (*I < *lastElPtr)
			std::iter_swap(I, border++);
	}
	
	std::iter_swap(border, lastElPtr);
	quickSort(begin, border);
	quickSort(border+1, end);
}

template<class T>
void quickInsertionSort(T *begin, T *end){
	if (end - begin <= 32){
		insertionSort(begin, end);
		return;
	}
	T *lastElPtr = end - 1;
	T *border = begin;
	for (T *I=begin; I<lastElPtr; ++I){
		if (*I < *lastElPtr)
		std::iter_swap(I, border++);
	}
	std::iter_swap(border, lastElPtr);
	quickSort(begin, border);
	quickSort(border+1, end);
}

namespace priv__{
	template<class T>
	void mergeRecursion(T * begin1, T * begin2, uint32_t length, bool swapFlag){
		if (length == 1){
			*begin2 = *begin1;
			return;
		}

		uint32_t halfLength = length>>1;
		mergeRecursion(begin1, begin2, halfLength, !swapFlag);
		mergeRecursion(begin1+halfLength, begin2+halfLength, length-halfLength, !swapFlag);

		if (swapFlag)
			std::iter_swap(&begin1, &begin2);

		T *I = begin1;
		T *L = begin2;
		begin1 = begin2 + length; // begin1 becomes end2
		begin2 += halfLength; // begin2 becomes mid2
		T *R = begin2;

		for (; L!=begin2 && R!=begin1; ++I)
			*I = *L<*R ? *(L++) : *(R++);
		for (; L!=begin2; ++I)
			*I = *(L++);
		for (; R!=begin1; ++I)
			*I = *(R++);
	}
}

template<class T>
inline void mergeSort(T *begin, T *end){
	uint32_t length = end - begin;
	T *tempArray = (T *)malloc(length*sizeof(T));
	priv__::mergeRecursion(begin, tempArray, length, false);
	free(tempArray);
}


template<class T>
inline void heapInsertionSort(T *begin, T *end){
	T *child;
	for (uint32_t i=(end-begin-1)>>1; i>=0; --i)
		for(uint32_t j=i; (child=begin+(j<<1)+1) < end; ){
			bool rightChildExists = child + 1 != end;
			if (child[0] >= begin[j] && (child[1] >= begin[j] || !rightChildExists))
				break;

			child += child[1] < child[0] && rightChildExists;

			std::iter_swap(begin+j, child);
			j = child - begin;
		}

	for (T *I=begin+1, *J, temp; I!=end; ++I){
		temp = *I;
		for(J=I-1; temp<*J; --J)
			*(J+1) = *J;
		*(J+1) = temp;
	}
}

template<class T>
void coctailShakerSort(T *begin, T *end){
	for (T *I; begin<end; ++begin, --end){
		for (I=begin+1; I<end; ++I)
			if (*I < *(I-1))
				std::iter_swap(I, I-1);

		for (I=end-2; I>begin; --I)
			if (*I < *(I-1))
				std::iter_swap(I, I-1);
	}
}

template<class T>
void shellSort(T *begin, T *end){
	for (uint32_t gap=(end-begin)>>1; gap!=0; gap>>=1)
		for (T *I=begin+gap, *J, temp; I!=end; ++I){
			temp = *I;
			for(J=I-gap; J>=begin && temp<*J; J-=gap)
				*(J+gap) = *J;
			*(J+gap) = temp;
		}
}

template<class T>
void combSort(T *begin, T *end){
	auto gapFunc = [](T &prevGap){
		return (prevGap*10)/13;
	};

	for (uint32_t gap=gapFunc(end-begin); gap!=0; gap=gapFunc(gap))
		for (T *I=begin; I!=end-gap; ++I)
			if (*(I+gap) < *I)
				std::iter_swap(I, I+gap);
}

template<class T>
void slowSort(T *begin, T *end){
	if (begin == end-1)
		return;

	T *mid = begin + ((end-begin)>>1);                            
	slowSort(begin, mid);
	slowSort(mid, end);
	
	T *lastElPtr = end - 1;
	if (*lastElPtr < *(mid-1))
		std::iter_swap(lastElPtr, mid-1);
	slowSort(begin, lastElPtr);
}


}	// END OF NAMESPACE	///////////////////////////////////////////////////////////////////
