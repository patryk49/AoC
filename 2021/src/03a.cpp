#include "SPL/Utils.hpp"

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

	uint32_t counters[12] = {0};
	uint32_t n = 0;

	do{
		for (size_t i=std::size(counters)-1; i!=(size_t)-1 && !feof(file); --i){
			const uint8_t c = getc(file) - '0';
			[[unlikely]] if (c > 1){
				fputs("wrong character\n", stderr);
				return 1;
			}
			counters[i] += c;
		}
		fscanf(file, " ");
		++n;
	} while (!feof(file));

	size_t gamma = 0;
	for (size_t i=std::size(counters)-1; i!=(size_t)-1; --i){
		gamma <<= 1;
		gamma |= n < 2*counters[i];
	}	

	const size_t epsilon = ~gamma & ((1<<std::size(counters))-1);
	
	printf("elements read: %llu\ngamma: %x\nepsilon: %x\npower: %u\n",
		n, gamma, epsilon, gamma*epsilon
	);
	
	fclose(file);
	return 0;
}
