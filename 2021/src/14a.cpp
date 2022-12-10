#include "SPL/Arrays.hpp"

#include "SPL/Scans.hpp"
#include "Error.hpp"




int main(int argc, char **argv){
	FILE *file;
	if (argc == 2){
		if (*argv[1] != 'c') sp::raiseError("wrong argument");
		file = stdin;
	} else{
		file = fopen("inputs/14.dat", "r");
		if (!file) sp::raiseError("file not found");
	}


	sp::DArray<char [3]> guides;
	sp::DArray<char> polym;
	polym.resize(256);

	fgets(std::begin(polym), 256, file);
	polym.resize(strlen(std::begin(polym))-1);

	if (getc(file) != '\n') sp::raiseError("missing newline\n");
	
	char buffer[16];
	for (;;){
		fgets(buffer, sizeof(buffer), file);
		if (buffer[0]<'A' || buffer[0]>'Z') break;

		guides.push_back();
		guides.back()[0] = buffer[0];
		guides.back()[1] = buffer[1];
		guides.back()[2] = buffer[6];
	}


	auto print = [&polym](){
		for (auto I : polym) putchar(I);
		putchar('\n');
	};	

	


	sp::DArray<char> polymSwap;
	for (size_t i=0; i!=10; ++i){
		std::swap(polym, polymSwap);
		polym.resize(0);

		polym.push_back(polymSwap.front());
		for (auto I=std::begin(polymSwap); ++I!=std::end(polymSwap);){
			for (auto J : guides)
				if (*(I-1)==J[0] && *I==J[1]){
					polym.push_back(J[2]);
					break;
				}
			polym.push_back(*I);
		}

//		print();	
	}

	uint32_t results[256];
	memset(results, 0, sizeof(results));

	for (auto I : polym) ++results[I];
	
	results[1] = UINT32_MAX;
	const uint32_t *minI = results+1;
	const uint32_t *maxI = results;
	
	for (auto I=std::begin(results)+2; I!=std::end(results); ++I){
		if (*I) minI = *minI>*I ? I : minI;
		maxI = *maxI<*I ? I : maxI;
	}

	printf(
		"least occuring character: %c\nmost accuring character: %c\npoints: %lu\n",
		(char)(minI-results), (char)(maxI-results), *maxI - *minI
	);

	return 0;
}
