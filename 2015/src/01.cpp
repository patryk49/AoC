#include "SPL/Arrays.hpp"


int main(){
	int64_t floor = 0;
	size_t len = 0;

	for (;; ++len){
		int c = getchar();
		if (c == ';') break;
		
		if (c == '(') ++floor;
		else if (c == ')') --floor;
	}

	printf("floor: %li\nlength: %lu\n", floor, len);
	
	return 0;
}
