#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "SPL/Allocators.hpp"
#include "SPL/Scans.hpp"
#include "Matrix.hpp"

MallocAllocator al = {};



int main(){
	DynamicArray<char> ans2 = {};

	size_t cycle = 1;
	int reg = 1;
	int ans1 = 0;
	
	size_t from = 20;
	size_t step = 40;

	int col = 0;

	for (;;){
		int c = getchar();
		for (size_t i=0; i!=4; i+=1) getchar();
		
		push_value(ans2, reg-1<=col && col<=reg+1 ? '#' : '.');
		col += 1;
		if (col >= (int)step){
			push_value(ans2, '\n');
			col = 0;
		}
		
		cycle += 1;

		if (cycle >= from){
			ans1 += from * reg;
			from += step;
		}
		
		if (c == 'a'){
			push_value(ans2, reg-1<=col && col<=reg+1 ? '#' : '.');
			col += 1;
			if (col >= (int)step){
				push_value(ans2, '\n');
				col = 0;
			}

			cycle += 1;
			
			auto[n, err, last] = read_data<int>(stdin);
			if (err || last != '\n') break;
			reg += n;
		} else if (c != 'n'){
			break;
		}
	}
	pop(ans2);

	printf("ans_1: %i\nans2:\n", ans1);
	for (char c : ans2) putchar(c);

	return 0;
}
