#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "SPL/Allocators.hpp"
#include "SPL/Scans.hpp"
#include "Matrix.hpp"

MallocAllocator al = {};


using Position = FixedVector<int, 2>;



int main(){
	auto[moves, err, _last] = read_array<Tuple<char, unsigned>>(stdin, al, '\n');
	if (err) return 1;

	DynamicArray<Position> visited;
	push_value(visited, Position{0, 0});

	Position head = {0, 0};
	Position tail = {0, 0};
	for (size_t i=0; i!=len(moves); i+=1){
		Position step;
		switch (get<0>(moves[i])){
		case 'U': step = Position{-1,  0}; break;
		case 'D': step = Position{ 1,  0}; break;
		case 'L': step = Position{ 0, -1}; break;
		case 'R': step = Position{ 0,  1}; break;
		default: return 1;
		}

		for (size_t j=0; j!=get<1>(moves[i]); j+=1){
			head += step;
			if (abs(head[0]-tail[0]) > 1 || abs(head[1]-tail[1]) > 1){
				tail = head - step;

				for (auto &it : visited) if (tail == it) goto Skip;
				push_value(visited, tail);
			Skip:;
			}
		}
	}

	size_t ans1 = len(visited);

	printf("ans_1: %lu\n", ans1);
//	printf("ans_2: %lu\n", ans2);

	return 0;
}
