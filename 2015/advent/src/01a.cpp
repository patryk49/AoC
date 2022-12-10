#include "SPL/Utils.hpp"

int main(){
	FILE *const file = fopen("inputs/01.dat", "r");
	if (!file){
		fputs(
			"could not find a file with name \"01.dat\" inside the"
				" directory inputs\n",
			stderr
		);
		return 1;
	}

	int32_t x;
	
	if (feof(file)){
		fputs("to few elements", stderr);
		return 1;
	}
	fscanf(file, "%d ", &x);
	if (feof(file)){
		fputs("to few elements", stderr);
		return 1;
	}

	uint32_t counter = 0;
	uint32_t progress = 1;

	do{
		const int32_t temp = x;
		fscanf(file, "%d ", &x);
		counter += x > temp;
		++progress;
	} while (!feof(file));
	
	printf("elements read: %d\ndepth increase: %d\n", progress ,counter);
	
	fclose(file);
	return 0;
}
