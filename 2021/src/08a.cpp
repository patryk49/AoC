#include "SPL/Arrays.hpp"
#include <stdio.h>
#include <array>

#include "Error.hpp"


int main(int argc, char **argv){
	FILE *file;
	if (argc == 2){
		if (*argv[1] != 'c') sp::raiseError("wrong argument");
		file = stdin;
	} else{
		file = fopen("inputs/08.dat", "r");
		if (!file) sp::raiseError("file not found");
	}

	sp::DArray<std::array<char[8], 10>> signals;
	sp::DArray<std::array<char[8], 4>> outputs;
	
	for (;;){
		signals.emplace_back();
		{
			auto &curr = signals.back();
			fscanf(file, "%[a-z] %[a-z] %[a-z] %[a-z] %[a-z] %[a-z] %[a-z] %[a-z] %[a-z] %[a-z] ",
				&curr[0], &curr[1], &curr[2], &curr[3], &curr[4],
				&curr[5], &curr[6], &curr[7], &curr[8], &curr[9]
			);
		}
		if (getc(file) != '|'){
			signals.pop_back();
			break;
		}
		outputs.emplace_back();
		{
			auto &curr = outputs.back();
			fscanf(file, " %[a-z] %[a-z] %[a-z] %[a-z]",
				&curr[0], &curr[1], &curr[2], &curr[3]
			);
		}
		if (getc(file) != '\n') break;
	}

	size_t trivials = 0;
	for (auto &I : outputs){
		for (size_t i=0; i!=4; ++i){
			const size_t len = strlen(I[i]);
			trivials += (len==2)|(len==3)|(len==4)|(len==7);
		}
	}

	printf("trivials:%8lu\n", trivials);

//	putchar('\n');
//	for (auto &I : signals){
//		for (auto &J : I) printf("%s ", &J);
//		putchar('\n');
//	}
//	putchar('\n');
//	for (auto &I : outputs){
//		for (auto &J : I) printf("%s ", &J);
//		putchar('\n');
//	}

	return 0;
}
