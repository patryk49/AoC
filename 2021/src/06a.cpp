#include "SPL/Scans.hpp"
#include "Error.hpp"


void update(sp::DArray<uint8_t> &fishes) noexcept{
	const size_t size = std::size(fishes);
	for (size_t i=0; i!=size; ++i){
		if (fishes[i] == 0){
			fishes[i] = 6;
			fishes.push_back(8);
		} else{
			--fishes[i];
		}
	}
}



int main(int argc, char **argv){
	size_t days = 80;
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


	for (size_t i=-1; i!=days;){
		++i;
//		printf("day%4lu   fish count = %lu\n", i, std::size(fishes));
		update(fishes);
	}

	printf("fish count after %lu days: %lu\n", days, std::size(fishes));

	return 0;
}
