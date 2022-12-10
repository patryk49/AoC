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

	int32_t x0;
	int32_t x1;
	int32_t x2;

	if (feof(file)){
		fputs("to few elements", stderr);
		return 1;
	}
	fscanf(file, "%d ", &x0);
	if (feof(file)){
		fputs("to few elements", stderr);
		return 1;
	}
	fscanf(file, "%d ", &x1);
	if (feof(file)){
		fputs("to few elements", stderr);
		return 1;
	}
	fscanf(file, "%d ", &x2);
	if (feof(file)){
		fputs("to few elements", stderr);
		return 1;
	}

	uint32_t counter = 0;
	uint32_t progress = 3;

	do{
		const int32_t temp = x0;
		x0 = x1;
		x1 = x2;
		fscanf(file, "%d ", &x2);
		counter += x2 > temp;
		++progress;
	} while (!feof(file));
	
	printf("elements read: %d\ndepth increase: %d\n", progress ,counter);
	
	fclose(file);
	return 0;
}
