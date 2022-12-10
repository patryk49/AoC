#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "SPL/Allocators.hpp"
#include "SPL/Scans.hpp"


MallocAllocator al;

int main(){
	Array<char> text = read_text(stdin, al);
	push_range(text, slice("\0\n\0\n\0\n\0\n\0\n\0\n\0"), al);


	size_t treshold = 100000;
	size_t ans1 = 0;


	Span<const char> prelude = slice("$ cd /\n");
	if (Span<char>{beg(text), 7} != prelude){
		puts("wrong start directory");
		return 1;
	}
	
	FiniteArray<size_t, 256> dirstack = {};
	DynamicArray<size_t> dirsizes = {};
	push_value(dirstack, 0);

	for (char *it=beg(text)+len(prelude);; it+=1){
		if (*it == '$'){
			it += 2;
			if (*it == 'l'){
				it += 2;
				continue;
			}
			
			it += 3;
			if (*it == '.'){
				it += 2;
				pop(dirstack);
				if (dirstack[len(dirstack)] <= treshold)
					ans1 += dirstack[len(dirstack)];
				dirstack[len(dirstack)-1] += dirstack[len(dirstack)];
				push_value(dirsizes, dirstack[len(dirstack)]);
			} else{
				while (*it != '\n') it += 1;
				push_value(dirstack, 0);
			}
			continue;
		}
		
		if (*it == 'd'){
			while (*it != '\n') it += 1;
			continue;
		}
		
		if ('0'>*it || *it>'9') break;
		
		dirstack[len(dirstack)-1] += strtol(it, &it, 10);
		while (*it != '\n') it += 1;
	}

	while (len(dirstack) != 0){
		pop(dirstack);
		if (dirstack[len(dirstack)] <= treshold)
			ans1 += dirstack[len(dirstack)];
		push_value(dirsizes, dirstack[len(dirstack)]);
		if (len(dirstack) != 0)
			dirstack[len(dirstack)-1] += dirstack[len(dirstack)];
	}


	size_t ans2 = SIZE_MAX;
	size_t tofree = 30'000'000 - (70'000'000 - dirstack[0]);
	for (size_t s : dirsizes)
		if (s >= tofree && s < ans2) ans2 = s;



	printf("ans_1: %lu\n", ans1);
	printf("ans_2: %lu\n", ans2);

	return 0;
}
