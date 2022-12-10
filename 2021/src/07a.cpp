#include "SPL/Scans.hpp"
#include "Error.hpp"


size_t consoomedFuel(const sp::DArray<uint32_t> &positions, const uint32_t destination) noexcept{
	size_t fuel = 0;
	for (auto I : positions) fuel += abs((int32_t)I - (int32_t)destination);
	return fuel;
}



int main(int argc, char **argv){
	FILE *file;

	switch (argc){
	case 1:
		file = fopen("inputs/07.dat", "r");
		if (!file) sp::raiseError("file not found\n");
		break;
	case 2:
		if (*argv[1] == 'c'){
			file = stdin;
			break;
		} else sp::raiseError("wrong argument\n");
	default: sp::raiseError("wrong number of arguments\n");
	}

	sp::DArray<uint32_t> positions;
	sp::scanArray(positions, file, ',');

	uint32_t dist = 0;
	size_t fuel = consoomedFuel(positions, dist);
	for (;; ++dist){
		const size_t newFuel = consoomedFuel(positions, dist+1);
		if (fuel <= newFuel) break;
		fuel = newFuel;
	}

	printf("optimal position:%8u\nrequired fuel:%11lu\n", dist, fuel);

	return 0;
}
