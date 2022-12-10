#include <stdio.h>
#include "SPL/Scans.hpp"


int main(){
	size_t points1 = 0;
	size_t points2 = 0;

	for (;;){
		auto[s1, err_s1, _0] = read_data<int>(stdin);
		if (err_s1) break;
		auto[e1, err_e1, _1] = read_data<int>(stdin);
		auto[s2, err_s2, _2] = read_data<int>(stdin);
		auto[e2, err_e2, _3] = read_data<int>(stdin);

		points1 += (s1>=s2 && e1<=e2) || (s2>=s1 && e2<=e1);
		points2 += s1<=e2 && s2<=e1;
	}
	

	printf("ans_1: %lu\n", points1);
	printf("ans_2: %lu\n", points2);


	return 0;
}
