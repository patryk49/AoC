#pragma once

#include "SPL/Utils.hpp"
#include <numeric>

namespace sp{ // BEGINING OF NAMESPACE //////////////////////////////////////////////////////////////////

template<class It>
void insertionSort(It begin, const It end);

template<class It, class Compare>
void insertionSort(It begin, const It end, Compare compare);

template<class It>
void heapSort(It begin, const It end);

template<class It, class Compare>
void heapSort(It begin, const It end, Compare compare);


template<class T>
void selectionSort(T *const begin, const T *const end);

template<class T>
void bubbleSort(T *const begin, const T *const end);

template<class T>
void gnomeSort(T *const begin, const T *const end);

template<class T>
void bogoSort(T *const begin, const T *const end);

template<class T>
void quickSort(T *const begin, const T *const end);

template<class T>
void quickInsertionSort(T *const begin, const T *const end);

template<class T>
inline void mergeSort(T *const begin, const T *const end);

template<class T>
void coctailShakerSort(T *begin, const T *end);

template<class T>
void shellSort(T *const begin, const T *const end);

template<class T>
void combSort(T *const begin, const T *const end);

template<class T>
void slowSort(T *const begin, const T *const end);





template<class It>
void insertionSort(It begin, const It end){
	typedef typename std::iterator_traits<It>::value_type Type;
	/* MAKE SURE THE FIRST ELEMENT IS A EXTREMUM */{	
		It min = begin;
		for (It I=begin+1; I<end; ++I)
			min = *I<*min ? I : min;
		sp::swap(begin, min);
	}	
	for (It I=begin+2; I<end; ++I){
		const Type temp = std::move(*I);
		It K = I;
		for (It J=I-1; temp<*J; --J){
			*K = std::move(*J);
			K = J;
		}
		*K = std::move(temp);
	}
}

template<class It, class Compare>
void insertionSort(It begin, const It end, Compare compare){
	typedef typename std::iterator_traits<It>::value_type Type;
	if (begin == end)
		return;
	if constexpr (compare(Type{}, Type{}) ||
		!std::is_same<typename std::iterator_traits<It>::iterator_category, typename std::random_access_iterator_tag>()
	){
		for (It I=begin+1; I!=end; ++I){
			const Type temp = std::move(*I);
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
			const Type temp = std::move(*I);
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
inline void heapSort(It begin, const It end){
	sp::makeHeap(begin, end);

	for (It I=(end)-1; I!=begin; --I){
		sp::swap(begin, I);
		sp::repairHeap(begin, I);
	}
}

template<class It, class Compare>
void heapSort(It begin, const It end, Compare compare){
	sp::makeHeap(begin, end, compare);

	for (It I=(end)-1; I!=begin; --I){
		sp::swap(begin, I);
		sp::repairHeap(begin, I, compare);
	}
}

template<class T>
void selectionSort(T *const begin, const T *const end){
	T *max;
	for (T *I=begin, *J; I!=end; ++I){
		for (J=I+1, max=I; J!=end; ++J)
			max = *max<*J ? max : J;
		sp::swap(I, max);
	}
}

template<class T>
void bubbleSort(T *const begin, const T *end){
	for (T *I; begin!=end; --end)
		for (I=begin+1; I<end; ++I)
			if (*I < *(I-1))
				sp::swap(I, I-1);
}

template<class T>
void gnomeSort(T *const begin, const T *const end){
	for (T *I=begin+1, *J; I!=end; ++I)
		for(J=I; J!=begin; --J){
			if (*(J-1) < *J)
				break;
			sp::swap(J, J-1);
		}
}

template<class T>
void bogoSort(T *const begin, const T *const end){
	while (true){
		std::random_shuffle(begin, end);
		for (T *I=begin+1; I!=end; ++I)
			if (*I < *(I-1))
				return;
	}
}

template<class T>
void quickSort(T *const begin, const T *const end){
	T *const lastElPtr = const_cast<T *>(end) - 1;
	if (begin >= lastElPtr)
		return;
	
	T *border = begin;
	for (T *I=begin; I<lastElPtr; ++I){
		if (*I < *lastElPtr)
			swap(I, border++);
	}
	
	swap(border, lastElPtr);
	quickSort(begin, border);
	quickSort(border+1, end);
}

template<class T>
void quickInsertionSort(T *const begin, const T *const end){
	if (end - begin <= 128){
		insertionSort(begin, end);
		return;
	}
	T *const lastElPtr = end - 1;
	T *border = begin;
	for (T *I=begin; I<lastElPtr; ++I){
		if (*I < *lastElPtr)
			swap(I, border++);
	}
	swap(border, lastElPtr);
	quickSort(begin, border);
	quickSort(border+1, end);
}

namespace priv__{
	template<class T>
	void mergeRecursion(T * begin1, T * begin2, const uint32_t length, const bool swapFlag){
		if (length == 1){
			*begin2 = *begin1;
			return;
		}

		const uint32_t halfLength = length>>1;
		mergeRecursion(begin1, begin2, halfLength, !swapFlag);
		mergeRecursion(begin1+halfLength, begin2+halfLength, length-halfLength, !swapFlag);

		if (swapFlag)
			sp::swap(&begin1, &begin2);

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
inline void mergeSort(T *const begin, const T *const end){
	const uint32_t length = end - begin;
	T *tempArray = (T *)malloc(length*sizeof(T));
	priv__::mergeRecursion(begin, tempArray, length, false);
	free(tempArray);
}


template<class T>
inline void heapInsertionSort(T *const begin, const T *const end){
	T *child;
	for (uint32_t i=(end-begin-1)>>1; i>=0; --i)
		for(uint32_t j=i; (child=begin+(j<<1)+1) < end; ){
			const bool rightChildExists = child + 1 != end;
			if (child[0] >= begin[j] && (child[1] >= begin[j] || !rightChildExists))
				break;

			child += child[1] < child[0] && rightChildExists;

			sp::swap(begin+j, child);
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
void coctailShakerSort(T *begin, const T *end){
	for (T *I; begin<end; ++begin, --end){
		for (I=begin+1; I<end; ++I)
			if (*I < *(I-1))
				sp::swap(I, I-1);

		for (I=end-2; I>begin; --I)
			if (*I < *(I-1))
				sp::swap(I, I-1);
	}
}

template<class T>
void shellSort(T *const begin, const T *const end){
	for (uint32_t gap=(end-begin)>>1; gap!=0; gap>>=1)
		for (T *I=begin+gap, *J, temp; I!=end; ++I){
			temp = *I;
			for(J=I-gap; J>=begin && temp<*J; J-=gap)
				*(J+gap) = *J;
			*(J+gap) = temp;
		}
}

template<class T>
void combSort(T *const begin, const T *const end){
	auto gapFunc = [](const T &prevGap){
		return (prevGap*10)/13;
	};

	for (uint32_t gap=gapFunc(end-begin); gap!=0; gap=gapFunc(gap))
		for (T *I=begin; I!=end-gap; ++I)
			if (*(I+gap) < *I)
				sp::swap(I, I+gap);
}

template<class T>
void slowSort(T *const begin, const T *const end){
	if (begin == end-1)
		return;

	T *const mid = begin + ((end-begin)>>1);                            
	slowSort(begin, mid);
	slowSort(mid, end);
	
	T *const lastElPtr = end - 1;
	if (*lastElPtr < *(mid-1))
		sp::swap(lastElPtr, mid-1);
	slowSort(begin, lastElPtr);
}


}	// END OF NAMESPACE	///////////////////////////////////////////////////////////////////