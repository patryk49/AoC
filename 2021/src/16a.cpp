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

	sp::DArray<Packet> packets;

	for (auto I=std::begin(bits); I!=std::end(bits);){
		Packet curr;

		curr.version = I[2];
		curr.version |= I[1] << 1;
		curr.version |= I[0] << 2;
		curr.type = I[5];
		curr.type |= I[4] << 1;
		curr.type |= I[3] << 2;
		I += 6;
		
		if (curr.type == 4){
			curr.data = 0;
			for (;;){
				curr.data |= I[4];
				curr.data |= I[3] << 1;
				curr.data |= I[2] << 2;
				curr.data |= I[1] << 3;
				
				bool continueFlag = I[0];
				I += 5;

				if (!continueFlag) break;
				curr.data <<= 4;
			}
		} else{
			if (I[0]){
				curr.data = (uint64_t)1 << 63;
				curr.data |= I[11] << 10;
				curr.data |= I[10] <<  9;
				curr.data |= I[ 9] <<  8;
				curr.data |= I[ 8] <<  7;
				curr.data |= I[ 7] <<  6;
				curr.data |= I[ 6] <<  5;
				curr.data |= I[ 5] <<  4;
				curr.data |= I[ 4] <<  3;
				curr.data |= I[ 3] <<  2;
				curr.data |= I[ 2] <<  1;
				curr.data |= I[ 1];
				I += 12;
			} else{
				curr.data = 0;
				curr.data |= I[15] << 14;
				curr.data |= I[14] << 13;
				curr.data |= I[13] << 12;
				curr.data |= I[12] << 11;
				curr.data |= I[11] << 10;
				curr.data |= I[10] <<  9;
				curr.data |= I[ 9] <<  8;
				curr.data |= I[ 8] <<  7;
				curr.data |= I[ 7] <<  6;
				curr.data |= I[ 6] <<  5;
				curr.data |= I[ 5] <<  4;
				curr.data |= I[ 4] <<  3;
				curr.data |= I[ 3] <<  2;
				curr.data |= I[ 2] <<  1;
				curr.data |= I[ 1];
				I += 16;	
			}		
		}

		if (I > std::end(bits)) break;
		packets.push_back(curr);
	}

	constexpr uint64_t countMask = ((uint64_t)1 << 63) - 1;

	size_t sum = 0;
	for (auto &I : packets) sum += I.version;

	printf("sum of versions: %lu\n", sum);

	return 0;
}
