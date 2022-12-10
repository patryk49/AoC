#include "SPL/Arrays.hpp"

#include "SPL/Scans.hpp"
#include "Error.hpp"


struct GridPosition{
	uint32_t row;
	uint32_t cols;
};



int main(int argc, char **argv){
	FILE *file;
	if (argc == 2){
		if (*argv[1] != 'c') sp::raiseError("wrong argument");
		file = stdin;
	} else{
		file = fopen("inputs/11.dat", "r");
		if (!file) sp::raiseError("file not found");
	}


	sp::DArray<uint8_t> energies;
	size_t rows = 1;
	size_t cols = 1;

	uint32_t c = getc(file) - '0';
	if (c > 9) sp::raiseError("missing data\n");
	for (;;){
		energies.push_back(c);
		c = getc(file) - '0';
		if (c > 9) break;
		++cols;
	}
	for (;;){
		if (c != (uint32_t)('\n'-'0')) break;
		++rows;
		for (size_t i=0; i!=cols; ++i) energies.push_back(getc(file)-'0');
		c = getc(file) - '0';
	}
	if (rows<2 || cols<2) sp::raiseError("too small dimansions\n");


	
	sp::DArray<GridPosition> quee;

	size_t i = 1;
	for (;; ++i){
		size_t flashes = 0;
		for (size_t rr=0; rr!=rows; ++rr)
			for (size_t cc=0; cc!=cols; ++cc){
				if (energies[rr*cols+cc] == 10) continue;

				quee.push_back(GridPosition{rr, cc});
				while (std::size(quee)){
					auto[r, c] = quee.front();
					std::move(std::begin(quee)+1, std::end(quee), std::begin(quee));
					quee.pop_back();
		
					const size_t index = r*cols + c;
					if (energies[index] == 10) continue;
					++energies[index];
					
					if (energies[index] == 10){
						++flashes;	
					
						if (r != 0 && c != 0)           quee.push_back(GridPosition{r-1, c-1});		
						if (r != 0)                     quee.push_back(GridPosition{r-1, c});		
						if (r != 0 && c != cols-1)      quee.push_back(GridPosition{r-1, c+1});		
						if (c != 0)                     quee.push_back(GridPosition{r, c-1});		
						if (c != cols-1)                quee.push_back(GridPosition{r, c+1});		
						if (r != rows-1 && c != 0)      quee.push_back(GridPosition{r+1, c-1});		
						if (r != rows-1)                quee.push_back(GridPosition{r+1, c});		
						if (r != rows-1 && c != cols-1) quee.push_back(GridPosition{r+1, c+1});		
					}
				}
			}
		if (flashes == rows*cols) break;

		for (auto &I : energies) I &= (I==10) - 1;

//		size_t counter = 0;
//		for (auto I : energies){
//			printf("%1u", (uint32_t)I);
//			if (++counter == cols){
//				putchar('\n');
//				counter = 0;
//			}
//		}
//		putchar('\n');
	}

	printf("full flash step: %lu\n", i);


	return 0;
}
