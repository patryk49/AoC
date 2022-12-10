#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "SPL/Allocators.hpp"
#include "SPL/Scans.hpp"


MallocAllocator al;

int main(){
	Array<char> text = read_text(stdin, al);
	
	size_t chars[256];
	memset(chars, 0, sizeof(chars));
	size_t code_len = 4;

	if (len(text) < code_len){
		puts("Pattern not found.");
		return 1;
	}

	size_t ans1;
	for (size_t i=0; i!=code_len; i+=1) chars[(size_t)text[i]] += 1;
	char last = text[0];

	for (size_t i=code_len;;){
		for (size_t j=0; j!=len(chars); j+=1) if (chars[j] > 1) goto Next1;
		ans1 = i;
		break;
	Next1:
		if (i == len(text)){
			puts("Pattern not found.");
			return 1;
		}

		chars[(size_t)last] -= 1;
		chars[(size_t)text[i]] += 1;

		i += 1;
		last = text[i-code_len];
	}
	
	memset(chars, 0, sizeof(chars));
	size_t code_len2 = 14;

	if (len(text) < code_len2){
		puts("Pattern not found.");
		return 1;
	}

	size_t ans2;
	for (size_t i=0; i!=code_len2; i+=1) chars[(size_t)text[i]] += 1;
	char last2 = text[0];

	for (size_t i=code_len2;;){
		for (size_t j=0; j!=len(chars); j+=1) if (chars[j] > 1) goto Next2;
		ans2 = i;
		break;
	Next2:
		if (i == len(text)){
			puts("Pattern not found.");
			return 1;
		}

		chars[(size_t)last2] -= 1;
		chars[(size_t)text[i]] += 1;

		i += 1;
		last2 = text[i-code_len2];
	}

	printf("ans_1: %lu\n", ans1);
	printf("ans_2: %lu\n", ans2);

	return 0;
}
