#include "SPL/Arrays.hpp"

#include "SPL/Scans.hpp"
#include "Error.hpp"




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


	auto getRisk = [rows, cols, &heights](const size_t r, const size_t c) noexcept{
		const size_t idx = r*cols + c;
		const uint8_t h = heights[idx];
		const bool cond = (r ? h<heights[idx-cols] : true)
		               && (r!=rows-1 ? h<heights[idx+cols] : true)
		               && (c ? h<heights[idx-1] : true)
		               && (c!=cols-1 ? h<heights[idx+1] : true);
//		if (cond) printf("lowpoint:%2u at [%3lu,%4lu]\n", h, r, c);
		return (h+1) & -cond;
	};



// CALCULATION
	size_t risk = 0;
	for (size_t i=0; i!=rows; ++i)
		for (size_t j=0; j!=cols; ++j)
			risk += getRisk(i, j);

	printf("risk: %lu\n", risk);

	return 0;
}
