#include "SPL/Utils.hpp"
#include "SPL/Arrays.hpp"

#include <algorithm>


int main(){
	FILE *const file = fopen("inputs/03.dat", "r");
	if (!file){
		fputs(
			"could not find a file with name \"03.dat\" inside the"
				" directory inputs\n",
			stderr
		);
		return 1;
	}

	
	if (feof(file)){
		fputs("to few elements", stderr);
		return 1;
	}

	constexpr size_t bits = 12;
	sp::DArray<uint32_t> oxygen;

	do{
		oxygen.push_back(0);
		for (size_t i=bits-1; i!=(size_t)-1 && !feof(file); --i){
			const uint8_t c = getc(file) - '0';
			[[unlikely]] if (c > 1){
				fputs("wrong character\n", stderr);
				return 1;
			}
			oxygen.back() |= c << i;
		}
		fscanf(file, " ");
	} while (!feof(file));
	sp::DArray<uint32_t> dioxide = oxygen;
	const size_t n = std::size(oxygen);


	{
		uint32_t bit = bits - 1;
		do{
			const uint32_t val = 2*std::count_if(
				std::begin(oxygen), std::end(oxygen),
				[bit](const uint32_t x){ return (x >> bit) & 1; }		
			) >= std::size(oxygen);
	
			oxygen.resize(std::remove_if(
				std::begin(oxygen), std::end(oxygen),
				[bit, val](const uint32_t x){ return ((x>>bit)&1) != val; }	
			) - std::begin(oxygen));
			--bit;
		} while (std::size(oxygen) != 1);	
	}

	{
		uint32_t bit = bits - 1;
		do{
			const uint32_t val = 2*std::count_if(
				std::begin(dioxide), std::end(dioxide),
				[bit](const uint32_t x){ return (x >> bit) & 1; }		
			) < std::size(dioxide);
	
			dioxide.resize(std::remove_if(
				std::begin(dioxide), std::end(dioxide),
				[bit, val](const uint32_t x){ return ((x>>bit)&1) != val; }	
			) - std::begin(dioxide));
			--bit;
		} while (std::size(dioxide) != 1);	
	}

	printf("elements read: %llu\noxygen: %u\ndioxide: %u\nsome rating: %u\n",
		n, oxygen.front(), dioxide.front(), oxygen.front()*dioxide.front()
	);
	
	fclose(file);
	return 0;
}
