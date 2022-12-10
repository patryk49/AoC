#include "SPL/Arrays.hpp"
#include <stdio.h>

#define SP_MATRIX_DEBUG
#include "matrix/MixedOperations.hpp"



void raiseError(const char *const msg) noexcept{
	fputs(msg, stderr);
	exit(1);
}


struct Line{
	uint32_t x0, x1;
	uint32_t y0, y1;
};


int main(){
	FILE *const file = fopen("inputs/05.dat", "r");
	[[unlikely]] if (!file)
		raiseError("cannot open a file \"05.dat\" form a directory\"inputs\"\n");

	sp::MatrixWrapper<sp::MatrixPoolAlloc<uint16_t, true>> grid{1000*1000};
	grid = sp::uniform(1000, 1000, 0);

	while (!feof(file)){
		uint32_t x0, x1;
		uint32_t y0, y1;
		if (fscanf(file, "%u%*c%u %*s %u%*c%u ", &x0, &y0, &x1, &y1) != 4)
			raiseError("too few arguments\n");

		if (x0 == x1){
			if (y1 < y0) std::swap(y0, y1);
			for (; y0<=y1; ++y0) ++grid(x0, y0);
		} else if (y0 == y1){
			if (x1 < x0) std::swap(x0, x1);
			for (; x0<=x1; ++x0) ++grid(x0, y0);
		} else if (abs((int)(x1-x0)) == abs((int)(y1-y0))){
			if (x0 < x1)
				if (y0 < y1)
					for (; x0<=x1; ++x0, ++y0) ++grid(x0, y0);
				else
					for (; x0<=x1; ++x0, --y0) ++grid(x0, y0);
			else
				if (y0 < y1)
					for (; x1<=x0; ++x1, --y1) ++grid(x1, y1);
				else
					for (; x1<=x0; ++x1, ++y1) ++grid(x1, y1);
		}
	}

//	for (size_t i=0; i!=sp::cols(grid); ++i){
//		for (size_t j=0; j!=sp::rows(grid); ++j)
//			printf("%2u", grid(j, i));
//		putchar('\n');
//	}
//	putchar('\n');

	size_t counter = 0;
	for (size_t i=0; i!=sp::rows(grid); ++i)
		for (size_t j=0; j!=sp::cols(grid); ++j)
			counter += grid(i, j) > 1;
	
	printf("count: %llu\n", counter);

	fclose(file);
	return 0;
}
