#include "SPL/Arrays.hpp"

#include "SPL/Scans.hpp"
#include "Error.hpp"



struct GridPosition{
	uint32_t row;
	uint32_t col;
};

size_t getBasinPoints(
	const sp::DArray<uint8_t> &heights, const size_t rows, const size_t cols
) noexcept{
	size_t basins[3] = {0};
	
	sp::DArray<bool> marks;
	marks.resize(rows*cols);
	for (auto &I : marks) I = false;

	sp::DArray<GridPosition> quee;
	
	size_t index = 0;
	for (size_t rr=0; rr!=rows; ++rr)
		for (size_t cc=0; cc!=cols; ++cc, ++index)
			if (!marks[index] && heights[index]!=9){
				size_t points = 0;
				quee.push_back(GridPosition{rr, cc});

				while (std::size(quee)){
					auto[r, c] = quee.front();
					std::move(std::begin(quee)+1, std::end(quee), std::begin(quee));
					quee.pop_back();
					
					const size_t idx = r*cols + c;
					if (marks[idx]) continue;
					
					marks[idx] = true;
					++points;

					if (r != 0) if(heights[idx-cols] != 9) quee.push_back(GridPosition{r-1, c});
					if (r != rows-1) if(heights[idx+cols] != 9) quee.push_back(GridPosition{r+1, c});
					if (c != 0) if(heights[idx-1] != 9) quee.push_back(GridPosition{r, c-1});
					if (c != cols-1) if(heights[idx+1] != 9) quee.push_back(GridPosition{r, c+1});
				}
					
				size_t *min = basins;
				min = *min>=basins[1] ? basins+1 : min;
				min = *min>=basins[2] ? basins+2 : min;
				
				if (points > *min) *min = points;
			}

	return basins[0] * basins[1] *basins[2];
}



int main(int argc, char **argv){
	FILE *file;
	if (argc == 2){
		if (*argv[1] != 'c') sp::raiseError("wrong argument");
		file = stdin;
	} else{
		file = fopen("inputs/09.dat", "r");
		if (!file) sp::raiseError("file not found");
	}


	sp::DArray<uint8_t> heights;
	size_t rows = 1;
	size_t cols = 1;

	uint32_t c = getc(file) - '0';
	if (c > 9) sp::raiseError("missing data\n");
	for (;;){
		heights.push_back(c);
		c = getc(file) - '0';
		if (c > 9) break;
		++cols;
	}
	for (;;){
		if (c != (uint32_t)('\n'-'0')) break;
		++rows;
		for (size_t i=0; i!=cols; ++i) heights.push_back(getc(file)-'0');
		c = getc(file) - '0';
	}
	if (rows<2 || cols<2) sp::raiseError("too small dimansions\n");


	printf("product: %lu\n", getBasinPoints(heights, rows, cols));

	return 0;
}
