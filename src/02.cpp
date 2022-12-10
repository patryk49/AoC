#include "stdio.h"


constexpr size_t point_table[3][3] = {
	{4, 1, 7},
	{8, 5, 2},
	{3, 9, 6}
};

constexpr size_t wins_table[3][3] = {
	{3, 1, 2},
	{4, 5, 6},
	{8, 9, 7}
};

int main(){
	size_t points1 = 0;
	size_t points2 = 0;

	for (;;){
		int c1 = getchar();
		if (c1!='A' && c1!='B' && c1!='C') break;
		getchar();
		int c2 = getchar();
		if (c2!='X' && c2!='Y' && c2!='Z') break;
		getchar();
		c1 -= 'A';
		c2 -= 'X';

		points1 += point_table[c2][c1];
		points2 += wins_table[c2][c1];
	}
	



	printf("ans_1: %lu\n", points1);
	printf("ans_2: %lu\n", points2);


	return 0;
}
