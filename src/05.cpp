#include <stdio.h>
#include <stdlib.h>

#include "SPL/Allocators.hpp"
#include "SPL/Scans.hpp"


MallocAllocator al;

int main(){
	FiniteArray<DynamicArray<char>, 32> stacks = {};

	Array<char> text = read_text(stdin, al);
	size_t letters = 0;
	for (; text[letters]!='\n'; letters+=1) if (text[letters]=='\0') return 1;
	letters += 1;
	size_t n_stacks = letters / 4;
	if (n_stacks*4 != letters) return 1;

	resize(stacks, n_stacks);
	size_t row = 0;
	for (;; row+=1){
		for (size_t i=0; i!=n_stacks; i+=1){
			char c = text[row*letters+1+i*4];
			if (c == ' ') continue;
			if ('A'>c || c>'Z') goto Finish;
			push_value(stacks[i], c);
		}
	}
Finish:
	for (auto &it : stacks){
		char *b = beg(it);
		char *e = end(it);
		for (; e-=1, b<e; b+=1) iswap(b, e);
	}

	FiniteArray<DynamicArray<char>, 32> stacks2 = {};
//	resize(stacks2, len(stacks));
//	for (size_t i=0; i!=len(stacks); i+=1){
//		resize(stacks2[i], len(stacks[i]));
//		for (size_t j=0; j!=len(stacks[i]); j+=1) stacks2[i][j] = stacks[i][j];
//	}
	stacks2 = stacks;

	for (const char *it = beg(text)+(row+1)*letters; *it=='\n' && *(it+1)=='m';){
		it += 6;
		size_t count = strtol(it, (char **)&it, 10);
		it += 6;
		size_t from = strtol(it, (char **)&it, 10) - 1;
		it += 4;
		size_t to = strtol(it, (char **)&it, 10) - 1;

		if (from == to) continue;
		for (size_t i=0; i!=count && !is_empty(stacks[from]); i+=1)
			push_value(stacks[to], pop_value(stacks[from]));
		
		if (count > len(stacks2[from])) count = len(stacks2[from]);
		shrink_back(stacks2[from], count);
		for (size_t i=0; i!=count; i+=1)
			push_value(stacks2[to], stacks2[from][len(stacks2[from])+i]);
	}
	
	printf("ans_1: ");
	for (auto &it : stacks) putchar(len(it)!=0 ? it[len(it)-1] : ' ');
	putchar('\n');
	printf("ans_2: ");
	for (auto &it : stacks2) putchar(len(it)!=0 ? it[len(it)-1] : ' ');
	putchar('\n');


	return 0;
}
