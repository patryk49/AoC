#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "SPL/Allocators.hpp"
#include "SPL/Scans.hpp"
#include "SPL/Hashmaps.hpp"
#include "Matrix.hpp"

MallocAllocator al = {};


using Position = FixedVector<int, 2>;

size_t position_hash(Position p) noexcept{ // this makes implementation 20x faster
	return ((p[0] + p[1]) * 33271821523 + 3271813);
}



int main(){
	auto[moves, err, _last] = read_array<Tuple<char, unsigned>>(stdin, al, '\n');
	if (err) return 1;

// PART 1
	FiniteHashset<Position, 8192, Position{INT32_MIN, INT32_MIN}, &position_hash> visited = {};

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
				add(visited, tail);
			}
		}
	}

	size_t ans1 = len(visited);


// PART 2
	reset(visited);
	Position knots[10];
	memset(beg(knots), 0, sizeof(knots));

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
			knots[0] += step;

			for (size_t k=1;  k!=len(knots); k+=1){
				if (abs(knots[k-1][0]-knots[k][0]) > 1 && abs(knots[k-1][1]-knots[k][1]) > 1){
					knots[k][0] = knots[k-1][0] + (knots[k-1][0]>knots[k][0] ? -1 : 1);
					knots[k][1] = knots[k-1][1] + (knots[k-1][1]>knots[k][1] ? -1 : 1);
				} else if (abs(knots[k-1][0]-knots[k][0]) > 1){
					knots[k][0] = knots[k-1][0] + (knots[k-1][0]>knots[k][0] ? -1 : 1);
					knots[k][1] = knots[k-1][1];
				} else if (abs(knots[k-1][1]-knots[k][1]) > 1){
					knots[k][0] = knots[k-1][0];
					knots[k][1] = knots[k-1][1] + (knots[k-1][1]>knots[k][1] ? -1 : 1);
				}
			}
			add(visited, knots[len(knots)-1]);
		}
	}

	size_t ans2 = len(visited);

	printf("ans_1: %lu\n", ans1);
	printf("ans_2: %lu\n", ans2);

	return 0;
}
