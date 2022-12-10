#include "SPL/Arrays.hpp"

#include "SPL/Scans.hpp"
#include "Error.hpp"

#include <unistd.h>


struct Packet{
	uint8_t type;
	uint8_t version;
	bool mode;
	uint64_t data;
};

struct ArgsInfo{
	size_t stop;
	uint32_t count;
	uint32_t root; 
};


struct Result{
	uint64_t value;
	size_t size;
};

#define print(s, x) \
{for(size_t i=0;i!=indent;++i){putchar(' ');putchar(' ');}printf("%s %lu\n",s,(uint64_t)x);}



Result solve(bool *iter, bool *end, size_t indent = 0) noexcept{
	if (iter == end) return Result{(uint64_t)-1, 0};
	
	bool *first = iter;

	uint8_t version = iter[2];
	version |= iter[1] << 1;
	version |= iter[0] << 2;
	uint8_t type = iter[5];
	type |= iter[4] << 1;
	type |= iter[3] << 2;

	iter += 6;

	if (type == 4){
		uint64_t value = 0;
		for (;;){
			value |= iter[4];
			value |= iter[3] << 1;
			value |= iter[2] << 2;
			value |= iter[1] << 3;
			
			bool continueFlag = iter[0];
			iter += 5;

			if (!continueFlag){
				print("val", value);
				return Result{value, iter-first};
			}
			value <<= 4;
		}
	} else{
		if (iter[0]){
			size_t args = iter[11];
			args |= iter[10] <<  1;
			args |= iter[ 9] <<  2;
			args |= iter[ 8] <<  3;
			args |= iter[ 7] <<  4;
			args |= iter[ 6] <<  5;
			args |= iter[ 5] <<  6;
			args |= iter[ 4] <<  7;
			args |= iter[ 3] <<  8;
			args |= iter[ 2] <<  9;
			args |= iter[ 1] << 10;
			
			iter += 12;
			
			switch (type){
			case 0: print("sum1", args); break;
			case 1: print("pro1", args); break;
			case 2: print("min1", args); break;
			case 3: print("max1", args); break;
			case 5: print("gre1", args); break;
			case 6: print("les1", args); break;
			case 7: print("equ1", args); break;
			}

			auto[value, size] = solve(iter, end, indent+1);
			iter += size;
			
			--args;
			switch (type){
			case 0:
				for (size_t i=0; i!=args; ++i){
					auto[res, size] = solve(iter, end, indent+1);
					[[unlikely]] if (iter+size >= end) break;
					value += res;
					iter += size;
				}
				break;
			case 1:
				for (size_t i=0; i!=args; ++i){
					auto[res, size] = solve(iter, end, indent+1);
					[[unlikely]] if (iter+size >= end) break;
					value *= res;
					iter += size;
				}
				break;
			case 2:
				for (size_t i=0; i!=args; ++i){
					auto[res, size] = solve(iter, end, indent+1);
					[[unlikely]] if (iter+size >= end) break;
					value = res<value ? res : value;
					iter += size;
				}
				break;
			case 3:
				for (size_t i=0; i!=args; ++i){
					auto[res, size] = solve(iter, end, indent+1);
					[[unlikely]] if (iter+size >= end) break;
					value = res>value ? res : value;
					iter += size;
				}
				break;
			case 5:{
				auto[res, size] = solve(iter, end, indent+1);
				value = value > res;
				iter += size;
			}
				break;
			case 6:{
				auto[res, size] = solve(iter, end, indent+1);
				value = value < res;
				iter += size;
			}
				break;
			case 7:{
				auto[res, size] = solve(iter, end, indent+1);
				value = value == res;
				iter += size;
			}
				break;
			default:
				 value = -1;
				 break;
			}
			return Result{value, iter-first};
		} else{
			size_t stopIndex = 0;
			stopIndex = iter[15];
			stopIndex |= iter[14] <<  1;
			stopIndex |= iter[13] <<  2;
			stopIndex |= iter[12] <<  3;
			stopIndex |= iter[11] <<  4;
			stopIndex |= iter[10] <<  5;
			stopIndex |= iter[ 9] <<  6;
			stopIndex |= iter[ 8] <<  7;
			stopIndex |= iter[ 7] <<  8;
			stopIndex |= iter[ 6] <<  9;
			stopIndex |= iter[ 5] << 10;
			stopIndex |= iter[ 4] << 11;
			stopIndex |= iter[ 3] << 12;
			stopIndex |= iter[ 2] << 13;
			stopIndex |= iter[ 1] << 14;			

			iter += 16;	
			bool *nextIter = iter + stopIndex;

			switch (type){
			case 0: print("sum0", stopIndex); break;
			case 1: print("pro0", stopIndex); break;
			case 2: print("min0", stopIndex); break;
			case 3: print("max0", stopIndex); break;
			case 5: print("gre0", stopIndex); break;
			case 6: print("les0", stopIndex); break;
			case 7: print("equ0", stopIndex); break;
			}

			auto[value, size] = solve(iter, end, indent+1);
			iter += size;
			switch (type){
			case 0:
				while (iter < nextIter){
					auto[res, size] = solve(iter, end, indent+1);
					value += res;
					iter += size;
				}
				break;
			case 1:
				while (iter < nextIter){
					auto[res, size] = solve(iter, end, indent+1);
					value *= res;
					iter += size;
				}
				break;
			case 2:
				while (iter < nextIter){
					auto[res, size] = solve(iter, end, indent+1);
					value = res<value ? res : value;
					iter += size;
				}
				break;
			case 3:
				while (iter < nextIter){
					auto[res, size] = solve(iter, end, indent+1);
					value = res>value ? res : value;
					iter += size;
				}
				break;
			case 5:{
				auto[res, size] = solve(iter, end, indent+1);
				value = value > res;
				iter += size;
			}
				break;
			case 6:{
				auto[res, size] = solve(iter, end, indent+1);
				value = value < res;
				iter += size;
			}
				break;
			case 7:{
				auto[res, size] = solve(iter, end, indent+1);
				value = value == res;
				iter += size;
			}
				break;
			default:
				value = -1;
				break;
			}
			return Result{value, nextIter-first};
		}
	}
}







int main(int argc, char **argv){
	FILE *file;
	if (argc == 2){
		if (*argv[1] != 'c') sp::raiseError("wrong argument");
		file = stdin;
	} else{
		file = fopen("inputs/16.dat", "r");
		if (!file) sp::raiseError("file not found");
	}


	sp::DArray<bool> bits;

	for (;;){
		int c = getc(file);
		if (!(c>='A' && c<='Z') && !(c>='0' && c<='9')) break;
		switch (c){
		case '0':
			bits.push_back(false);
			bits.push_back(false);
			bits.push_back(false);
			bits.push_back(false);
			break;
		case '1':
			bits.push_back(false);
			bits.push_back(false);
			bits.push_back(false);
			bits.push_back(true);
			break;
		case '2':
			bits.push_back(false);
			bits.push_back(false);
			bits.push_back(true);
			bits.push_back(false);
			break;
		case '3':
			bits.push_back(false);
			bits.push_back(false);
			bits.push_back(true);
			bits.push_back(true);
			break;
		case '4':
			bits.push_back(false);
			bits.push_back(true);
			bits.push_back(false);
			bits.push_back(false);
			break;
		case '5':
			bits.push_back(false);
			bits.push_back(true);
			bits.push_back(false);
			bits.push_back(true);
			break;
		case '6':
			bits.push_back(false);
			bits.push_back(true);
			bits.push_back(true);
			bits.push_back(false);
			break;
		case '7':
			bits.push_back(false);
			bits.push_back(true);
			bits.push_back(true);
			bits.push_back(true);
			break;
		case '8':
			bits.push_back(true);
			bits.push_back(false);
			bits.push_back(false);
			bits.push_back(false);
			break;
		case '9':
			bits.push_back(true);
			bits.push_back(false);
			bits.push_back(false);
			bits.push_back(true);
			break;
		case 'A':
			bits.push_back(true);
			bits.push_back(false);
			bits.push_back(true);
			bits.push_back(false);
			break;
		case 'B':
			bits.push_back(true);
			bits.push_back(false);
			bits.push_back(true);
			bits.push_back(true);
			break;
		case 'C':
			bits.push_back(true);
			bits.push_back(true);
			bits.push_back(false);
			bits.push_back(false);
			break;
		case 'D':
			bits.push_back(true);
			bits.push_back(true);
			bits.push_back(false);
			bits.push_back(true);
			break;
		case 'E':
			bits.push_back(true);
			bits.push_back(true);
			bits.push_back(true);
			bits.push_back(false);
			break;
		case 'F':
			bits.push_back(true);
			bits.push_back(true);
			bits.push_back(true);
			bits.push_back(true);
			break;
		default: sp::raiseError("unrecognized character\n");
		}
	}

	printf("result: %llu\n", solve(std::begin(bits), std::end(bits)).value);

	return 0;
}
