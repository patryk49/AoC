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
		file = fopen("inputs/17.dat", "r");
		if (!file) sp::raiseError("file not found");
	}

	
	int x0, x1, y0, y1;
	if (fscanf(file, "%i %i %i %i", &x0, &x1, &y0, &y1) != 4)
		sp::raiseError("wrong argument format\n");


	size_t hits = 0;

	puts("computing in progress:");

	for (int vx0=0; vx0<=x1; ++vx0){
		for (int vy0=y0; vy0<=-y0; ++vy0){
			int vx = vx0;
			int vy = vy0;

			int x = 0;
			int y = 0;
			for (; x<=x1 && y>=y0; vx-=vx!=0, --vy){
				x += vx;
				y += vy;
				if (x0<=x && x<=x1 && y0<=y && y<=y1){
					printf("vx=%5i   vy=%5i\n", vx0, vy0);
					++hits;
					break;;
				}
			}
		}
	}

	printf("hits: %lu\n", hits);

	return 0;
}
