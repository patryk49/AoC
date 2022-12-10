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

	
	int32_t x0, x1, y0, y1;
	if (fscanf(file, "%i %i %i %i", &x0, &x1, &y0, &y1)) sp::raiseError("wrong argument format\n");

	



	return 0;
}
