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



	
	constexpr uint8_t sent = 'Z' + 1;

	uint64_t charCounts[sent-'A'];
	memset(charCounts, 0, sizeof(charCounts));
	for (auto I : polym) ++charCounts[I-'A'];




	uint8_t map[sent-'A'][sent-'A'];
	
	for (uint8_t i=0; i<='Z'-'A'; ++i){
		for (uint8_t j=0; j<='Z'-'A'; ++j){
			for (auto &I : guides)
				if (i==I[0]-'A' && j==I[1]-'A'){
					map[i][j] = I[2]-'A';
					goto Skip0;
				}
			map[i][j] = sent;
		Skip0:;
		}
	}

	size_t pairs[sent-'A'][sent-'A'];
	memset(pairs, 0, sizeof(pairs));

	for (auto I=std::begin(polym); ++I!=std::end(polym);)
		if (map[*(I-1)-'A'][*I-'A'] != sent) ++pairs[*(I-1)-'A'][*I-'A'];


	auto print = [&charCounts](){
		uint64_t min = UINT64_MAX;
		uint64_t max = 0;
		for (size_t i=0; i!=std::size(charCounts); ++i){
			if (charCounts[i]) min = min>charCounts[i] ? charCounts[i] : min;
			max = max<charCounts[i] ? charCounts[i] : max;
		}
		printf("min: %llu\nmax: %llu\npoints: %llu\n", min, max, max-min);
		
		uint64_t total = 0;
		uint64_t nonZero = 0;
		for (auto I=std::begin(charCounts); I!=std::end(charCounts); ++I){
			total += *I;
			nonZero += *I != 0;
		}
		printf("total count: %llu\noccuring characters: %llu\n", total, nonZero);
	};


	size_t pairsCopy[sent-'A'][sent-'A'];

	for (size_t i=0; i!=40; ++i){
//		print();
		memcpy(pairsCopy, pairs, sizeof(pairs));

		for (uint8_t j=0; j<='Z'-'A'; ++j)
			for (uint8_t k=0; k<='Z'-'A'; ++k){
				const uint8_t ins = map[j][k];
				if (ins == sent) continue;

				const uint64_t count = pairsCopy[j][k];				
				charCounts[ins] += count;
				
				pairs[j][k] -= count;
				pairs[j][ins] += (map[j][ins] != sent) * count;
				pairs[ins][k] += (map[ins][k] != sent) * count;
			}
	}	

	print();

	return 0;
}
