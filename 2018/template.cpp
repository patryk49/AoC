#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include <SPL/Utils.hpp>
#include <SPL/Allocators.hpp>
#include <SPL/Arrays.hpp>
#include <SPL/Scans.hpp>


int main(){
	int64_t sum = 0;

	for (;;){
		auto [n, term, err] = scan_number<int>(stdin);
		assert(term == '\n');
		if (err) break;
		sum += n;
	}

	printf("\nresult: %li\n", sum);

	return 0;
}
