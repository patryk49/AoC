#include "SPL/Arrays.hpp"


int main(){
	int64_t floor = 0;

	char buff[20000];
	fgets(buff, std::size(buff), stdin);


	for (char *c=buff; *c!='\0'; ++c){
		if (*c == '(') ++floor;
		else if (*c == ')') --floor;
		
		if (floor < 0){
			printf("len: %lu\n", c-buff);
			return 0;
		}
	}
}
