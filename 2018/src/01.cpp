#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include <SPL/Utils.hpp>
#include <SPL/Allocators.hpp>
#include <SPL/Arrays.hpp>
#include <SPL/Scans.hpp>


int main(){
// INPUT
	DynamicArray<int, MallocAllocator<>> freqs;
	scan_array(freqs, stdin, '\n');

// PART 1
	int64_t sum = 0;
	for (size_t i=0; i!=len(freqs); ++i) sum += freqs[i];
	printf("\npart_1: %li\n", sum);

// PART 2
	int64_t first = 0;
	DynamicArray<int64_t, MallocAllocator<>> reached;
	
	for (;;) for (size_t i=0; i!=len(freqs); ++i){
		for (size_t j=0; j!=len(reached); ++j) if (first == reached[j]) goto Break;
		push_value(reached, first);
		first += freqs[i];
	}
Break:
	printf("\npart_2: %li\n", first);

	return 0;
}
