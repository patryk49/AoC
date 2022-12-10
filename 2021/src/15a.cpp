#include "SPL/Arrays.hpp"

#include "SPL/Scans.hpp"
#include "Error.hpp"

#include <unistd.h>

struct Position{
	uint32_t row;
	uint32_t col;
	uint32_t dist;
};



uint32_t findPath(sp::DArray<uint8_t> &risks, size_t rows, size_t cols) noexcept{
	sp::DArray<uint32_t> costs(rows*cols);
	for (auto &I : costs) I = UINT32_MAX;
	costs[0] = 0;
	costs[1] = risks[1];
	costs[cols] = risks[cols];

	sp::DArray<Position> pquee;

	auto pqueeInsert = [&pquee, cols](uint32_t r, uint32_t c, uint32_t d) noexcept{
		pquee.push_back();

		auto I = std::end(pquee)-1;
		while (I->dist<d && I!=std::begin(pquee)){
			*I = *(I-1);
			--I;
		}
		I->row = r;
		I->col = c;
		I->dist = d;
	};

	pquee.push_back(Position{0, 1, risks[1]});
	pqueeInsert(1, 0, risks[cols]);
	
	while (std::size(pquee)){
		pquee.pop_back();
		Position curr = *std::end(pquee);
		size_t index = curr.row*cols + curr.col;
		if (costs[index] < curr.dist) continue;
		
		if (curr.row != 0){
			size_t neigIndex = index - cols;	
			if (costs[neigIndex]){
				uint32_t newCost = costs[index] + risks[neigIndex];
				if (newCost < costs[neigIndex]){
					costs[neigIndex] = newCost;
					pqueeInsert(curr.row-1, curr.col, newCost);
				}
			}
		}
	
		if (curr.row != rows-1){
			size_t neigIndex = index + cols;	
			if (costs[neigIndex]){
				uint32_t newCost = costs[index] + risks[neigIndex];
				if (newCost < costs[neigIndex]){
					costs[neigIndex] = newCost;
					pqueeInsert(curr.row+1, curr.col, newCost);
				}
			}
		}
	
		if (curr.col != 0){
			size_t neigIndex = index - 1;	
			if (costs[neigIndex]){
				uint32_t newCost = costs[index] + risks[neigIndex];
				if (newCost < costs[neigIndex]){
					costs[neigIndex] = newCost;
					pqueeInsert(curr.row, curr.col-1, newCost);
				}
			}
		}
	
		if (curr.col != cols-1){
			size_t neigIndex = index + 1;	
			if (costs[neigIndex]){
				uint32_t newCost = costs[index] + risks[neigIndex];
				if (newCost < costs[neigIndex]){
					costs[neigIndex] = newCost;
					pqueeInsert(curr.row, curr.col+1, newCost);
				}
			}
		}
	}
	return costs[rows*cols-1];
}



int main(int argc, char **argv){
	FILE *file;
	if (argc == 2){
		if (*argv[1] != 'c') sp::raiseError("wrong argument");
		file = stdin;
	} else{
		file = fopen("inputs/15.dat", "r");
		if (!file) sp::raiseError("file not found");
	}


	sp::DArray<uint8_t> risks;
	size_t rows = 1;
	size_t cols = 1;

	uint32_t c = getc(file) - '0';
	if (c > 9) sp::raiseError("missing data\n");
	for (;;){
		risks.push_back(c);
		c = getc(file) - '0';
		if (c > 9) break;
		++cols;
	}
	for (;;){
		if (c != (uint32_t)('\n'-'0')) break;
		++rows;
		for (size_t i=0; i!=cols; ++i) risks.push_back(getc(file)-'0');
		c = getc(file) - '0';
	}
	if (rows<2 || cols<2) sp::raiseError("too small dimansions\n");




	printf("\nshortest path: %u\n", findPath(risks, rows, cols));

	return 0;
}
