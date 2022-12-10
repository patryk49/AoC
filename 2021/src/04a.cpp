#include "SPL/Utils.hpp"
#include "SPL/Arrays.hpp"


struct Bingo{
	uint32_t marks;
	uint8_t data[5][5];
};



void raiseError(const char *const msg){
	fputs(msg, stderr);
	exit(1);
}



int main(){
	FILE *const file = fopen("inputs/04.dat", "r");
	if (!file) raiseError("no file \"04.dat\" in a directory\"inputs\"\n");

	sp::DArray<uint8_t> bets;
	do {
		[[unlikely]] if (feof(file)) raiseError("missing data\n");
		uint32_t x;
		[[unlikely]] if (!fscanf(file, " %u", &x)) raiseError("wrong character\n");
		bets.push_back(x);
	} while (getc(file) != '\n');


	sp::DArray<Bingo> tabs;
	while (getc(file) == '\n'){
		tabs.emplace_back();
		tabs.back().marks = 0;
		for (size_t i=0; i!=5; ++i){
			for (size_t j=0; j!=5; ++j)
				[[unlikely]] if (!fscanf(file, " %5d", &tabs.back().data[i][j]))
					raiseError("too few columns");
			[[unlikely]] if (getc(file) != '\n') raiseError("too few rows");
		}
	}


	const Bingo *result;
	uint32_t lastNumber;

	for (auto I : bets)
		for (auto &J : tabs)
			for (size_t i=0; i!=5; ++i)
				for (size_t j=0; j!=5; ++j)
					if (J.data[i][j] == I){
						J.marks |= 1 << (i*5 + j);
						constexpr uint32_t rowMask = 0b0000100001000010000100001;
						constexpr uint32_t colMask = 0b0000000000000000000011111;
						if (
							(((J.marks >> j) & rowMask) == rowMask)
							| (((J.marks >> i*5) & colMask) == colMask)
						){
							result = &J;
							lastNumber = I;
							goto Break;
						}
					}
	raiseError("something went wrong\n");
Break:
	
	for (size_t i=0; i!=5; ++i){
		for (size_t j=0; j!=5; ++j)
			printf(
				(result->marks >> (i*5+j))&1 ? "[%2u]" : " %2u ",
				(uint32_t)result->data[i][j]
			);
		putchar('\n');
	}
	putchar('\n');

	uint32_t sum = 0;
	for (size_t i=0; i!=5; ++i)
		for (size_t j=0; j!=5; ++j)
			sum += result->data[i][j] & (((result->marks >> (i*5 + j))&1) - 1);

	printf("last number: %u\nsum of unmarked: %u\nproduct: %u\n", lastNumber, sum, sum*lastNumber);

	return 0;
}
