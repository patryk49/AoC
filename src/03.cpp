#include "stdlib.h"


#include "SPL/Utils.hpp"
#include "SPL/Allocators.hpp"
#include "SPL/Arrays.hpp"
#include "SPL/Scans.hpp"
#include <algorithm>
#include <array>

size_t priority(char c) noexcept{
	return 'a'<=c && c<='z' ? 1+c-'a' : 27+c-'A';
}


int main(){
	DynamicArray<DynamicArray<char>> data{};

	for (;;){
		int c = getchar();
		if (('a'>c || c>'z') && ('A'>c || c>'Z')) break;
		push_value(data, DynamicArray<char>{});

		for (;;){
			push_value(data[len(data)-1], c);
			
			c = getchar();
			if (('a'>c || c>'z') && ('A'>c || c>'Z')) break;
		}
	}

	size_t sum1 = 0;
	for (auto &it : data){
		size_t mid = len(it)/2;
		for (size_t i=0; i!=mid; i+=1)
			for (size_t j=mid; j!=len(it); j+=1)
				if (it[i] == it[j]){
					sum1 += priority(it[j]);
					goto Break1;
				}
	Break1:;
	}
	
	if (len(data )% 3 != 0){ puts("input size is not multiple of 3"); return 1; }
	size_t sum2 = 0;
	for (size_t i=0; i!=len(data); i+=3){
		for (auto a : data[i+0])
			for (auto b : data[i+1])
				if (a == b){
					for (auto c : data[i+2])
						if (b == c){
							sum2 += priority(c);
							goto Break2;
						}
				}
	Break2:;
	}

	printf("ans_1: %lu\n", sum1);
	printf("ans_2: %lu\n", sum2);

	return 0;
}
