#include "SPL/Utils.hpp"


int main(){
	FILE *const file = fopen("inputs/02.dat", "r");
	if (!file){
		fputs("there's no file \"02.dat\" i a directory \"inputs\"", stderr);
		return 1;
	}


	int32_t posX = 0;
	int32_t posY = 0;
	int32_t aim = 0;

	char buffer[8];
	while (!feof(file)){
		int32_t num;
		fscanf(file, "%7s %d ", buffer, &num);
		if (!strcmp(buffer, "down")){
			aim += num;
		} else if (!strcmp(buffer, "up")){
			aim -= num;
		} else if (!strcmp(buffer, "forward")){
			posX += num;
			posY += aim * num;
		} else{
			fputs("wrong keyword\n", stderr);
			return 1;
		}
	}

	printf("depth: %d\ndistance: %d\nproduct: %d\n", posX, posY, posX*posY);

	fclose(file);
	return 0;
}
