#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "SPL/Allocators.hpp"
#include "SPL/Scans.hpp"


MallocAllocator al = {};


int main(){
	auto[trees, nrows, ncols, err1, err2, _last] = read_matrix<uint8_t>(stdin, al);
	if (err1 || err2) return 1;
	if (nrows != ncols) return 1;

	auto max_up    = Span<uint8_t>{(uint8_t *)alloc(al, len(trees)).ptr, len(trees)};
	auto max_down  = Span<uint8_t>{(uint8_t *)alloc(al, len(trees)).ptr, len(trees)};
	auto max_left  = Span<uint8_t>{(uint8_t *)alloc(al, len(trees)).ptr, len(trees)};
	auto max_right = Span<uint8_t>{(uint8_t *)alloc(al, len(trees)).ptr, len(trees)};

	for (size_t i=0; i!=len(trees); i+=1){
		max_up[i]    = trees[i];
      max_down[i]  = trees[i];
      max_left[i]  = trees[i];
      max_right[i] = trees[i];
	}
	
	size_t ans1 = 2*nrows + 2*ncols - 4;
	
	size_t last = nrows - 1;
	for (size_t i=1; i!=last; i+=1){
		for (size_t j=0; j!=nrows; j+=1){
			if (max_up[(i-1)*ncols+j] > max_up[(i)*ncols+j])
				max_up[(i)*ncols+j] = max_up[(i-1)*ncols+j];
			
			if (max_down[(nrows-i)*ncols+j] > max_down[(last-i)*ncols+j])
				max_down[(last-i)*ncols+j] = max_down[(nrows-i)*ncols+j];
			
			if (max_left[j*ncols+(i-1)] > max_left[j*ncols+(i)])
				max_left[j*ncols+(i)] = max_left[j*ncols+(i-1)];
			
			if (max_right[j*ncols+(nrows-i)] > max_right[j*ncols+(last-i)])
				max_right[j*ncols+(last-i)] = max_right[j*ncols+(nrows-i)];
		}
	}

	size_t ans2 = 0;

	for (size_t i=1; i!=last; i+=1){
		for (size_t j=1; j!=last; j+=1){
			uint8_t curr = trees[i*ncols+j];
			if (
				curr >    max_up[(i-1)*ncols+j] ||
				curr >  max_down[(i+1)*ncols+j] ||
				curr >  max_left[i*ncols+(j-1)] ||
				curr > max_right[i*ncols+(j+1)]
			){
				ans1 += 1;
				
				size_t up = 0;
				size_t down = 0;
				size_t left = 0; 
				size_t right = 0;
				for (size_t k=i; k-=1, k!=SIZE_MAX;){ up += 1; if (trees[k*ncols+j] >= curr) break; }
				for (size_t k=i; k+=1, k!=nrows;){ down += 1; if (trees[k*ncols+j] >= curr) break; }
				for (size_t k=j; k-=1, k!=SIZE_MAX;){ left += 1; if (trees[i*ncols+k] >= curr) break; }
				for (size_t k=j; k+=1, k!=nrows;){ right += 1; if (trees[i*ncols+k] >= curr) break; }

				size_t score = up * down * left * right;
				if (score > ans2) ans2 = score;
			}
		}
	}

	printf("ans_1: %lu\n", ans1);
	printf("ans_2: %lu\n", ans2);

	return 0;
}
