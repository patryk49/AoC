#include "SPL/Arrays.hpp"
#include <stdio.h>
#include <array>

#include "Error.hpp"



uint8_t toBits(const char *code) noexcept{
	uint8_t result = 0;
	for (; *code; ++code) result |= 1 << (*code - 'a');
	return result;
}



int main(int argc, char **argv){
	FILE *file;
	if (argc == 2){
		if (*argv[1] != 'c') sp::raiseError("wrong argument");
		file = stdin;
	} else{
		file = fopen("inputs/08.dat", "r");
		if (!file) sp::raiseError("file not found");
	}

	
	size_t sum = 0;

	uint8_t nums[10];
	constexpr uint8_t mask = 0x7f;
	nums[8] = mask;
	for (;;){
		uint8_t d5[3]; // 2 3 5
		uint8_t d6[3]; // 0 6 9
		uint8_t d5len = 0;
		uint8_t d6len = 0;
		
		for (size_t i=0; i!=10; ++i){
			char buff[8];
			fscanf(file, "%[a-z] ", buff);
			uint8_t code = toBits(buff);
			
			switch (strlen(buff)){
			case 2:
				nums[1] = code;
				break;
			case 3:
				nums[7] = code;
				break;
			case 4:
				nums[4] = code;
				break;
			case 5:
				d5[d5len] = code;
				++d5len;
				break;
			case 6:
				d6[d6len] = code;
				++d6len;
				break;
			case 7: break;
			default: sp::raiseError("signal code has too many characters\n");
			}
		}
		if (getc(file) != '|') break;

		// SOLVING
		const uint8_t right = nums[1];
		nums[3] = (d5[0] & -((d5[0]&right)==right))
		        | (d5[1] & -((d5[1]&right)==right))
		        | (d5[2] & -((d5[2]&right)==right));

		nums[6] = (d6[0] & (((d6[0]&right)==right)-1))
		        | (d6[1] & (((d6[1]&right)==right)-1))
		        | (d6[2] & (((d6[2]&right)==right)-1));

		const uint8_t top = nums[7] & ~nums[1];
		const uint8_t bottom = nums[3] & ~nums[7] & ~nums[4];
		const uint8_t left = ~nums[3] & mask;
		
		nums[0] = nums[7] | left | bottom;
		nums[5] = (nums[4] & nums[6]) | top | bottom;
		nums[9] = nums[5] | right;
		nums[2] = nums[5] ^ right ^ left;


		{
			char outputs[4][8];
			fscanf(file, " %[a-z] %[a-z] %[a-z] %[a-z]",
				&outputs[0], &outputs[1], &outputs[2], &outputs[3]
			);
			
			size_t multiplier = 1;
			for (size_t i=std::size(outputs)-1; i!=(size_t)-1; --i){
				const uint8_t code = toBits(outputs[i]);

				for (size_t i=0; i!=std::size(nums); ++i)
					if (nums[i] == code){
						sum += i * multiplier;
						goto Break;
					}
				sp::raiseError("decoding error\n");
			Break:
				multiplier *= 10;
			}
		}

		if (getc(file) != '\n') break;
	}


	printf("sum:%8lu\n", sum);

	return 0;
}
