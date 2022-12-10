#include "SPL/Scans.hpp"
#include "Error.hpp"


void update(size_t (&counts)[9]) noexcept{
	const size_t babyCount = counts[0];
	counts[0] = counts[1];
	counts[1] = counts[2];
	counts[2] = counts[3];
	counts[3] = counts[4];
	counts[4] = counts[5];
	counts[5] = counts[6];
	counts[6] = counts[7] + babyCount;
	counts[7] = counts[8];
	counts[8] = babyCount;
}



int main(int argc, char **argv){
	size_t days = 256;
	FILE *file;

	switch (argc){
	case 2:
		days = strtol(argv[1], nullptr, 10);
		if (!days) sp::raiseError("wrong number of days\n");
	case 1:
		file = fopen("inputs/06.dat", "r");
		if (!file) sp::raiseError("file not found\n");
		break;
	case 3:
		if (*argv[1] == 'c'){
			file = stdin;
			days = strtol(argv[2], nullptr, 10);
			if (!days) sp::raiseError("wrong number of days\n");
			break;
		} else sp::raiseError("wrong argument\n");
	default: sp::raiseError("wrong number of arguments\n");
	}

	sp::DArray<uint8_t> fishes;
	sp::scanArray(fishes, file, ',');
	
	printf("day   0   fish count = %lu\n", std::size(fishes));

	size_t counts[9] = {};
	for (auto I : fishes) ++counts[I];

	for (size_t i=0; i!=days;){
		++i;
		update(counts);
		size_t sum = 0;
		for (auto I : counts) sum += I;
//		printf("day%4lu   fish count = %lu\n", i, sum);
	}

	size_t sum = 0;
	for (auto I : counts) sum += I;
	printf("fish count after %lu days: %lu\n", days, sum);

	return 0;
}
