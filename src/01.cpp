#include "stdlib.h"


#include "SPL/Utils.hpp"
#include "SPL/Allocators.hpp"
#include "SPL/Arrays.hpp"
#include "SPL/Scans.hpp"
#include <algorithm>
#include <array>

MallocAllocator al;


int main(){
	DynamicArray<int> arr{};

	for (;;){
		auto[data, _, _l] = read_array<int>(stdin, al, '\n');
		if (len(data) == 0) break;
		int sum = 0;
		for (int i : data) sum += i;
		push_value(arr, sum);
		deinit(data, al);
	}

	int m = 0; for (int i : arr) if (i > m) m = i;
	printf("ans_1: %i\n", m);
	
	int ms[3] = {0, 0, 0};
	for (int i : arr){
		if (ms[0] < i){
			size_t j = 1;
			for (; ms[j]<i && j!=len(ms); j+=1) ms[j-1] = ms[j];
			ms[j-1] = i;
		}
	}

	int msum = 0; for (int i : ms) msum += i;

	printf("ans_2: %i\n", msum);


	return 0;
}
