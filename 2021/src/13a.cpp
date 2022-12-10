#include "SPL/Arrays.hpp"

#include "SPL/Scans.hpp"
#include "Error.hpp"




int main(int argc, char **argv){
	FILE *file;
	if (argc == 2){
		if (*argv[1] != 'c') sp::raiseError("wrong argument");
		file = stdin;
	} else{
		file = fopen("inputs/13.dat", "r");
		if (!file) sp::raiseError("file not found");
	}


	sp::DArray<int32_t [2]> heats;
	sp::DArray<int32_t> folds;

	constexpr uint32_t cordshift = 31;
	constexpr uint32_t cordpos = 1 << cordshift; // last bit indicates the coordinate
	constexpr uint32_t foldmask = cordpos - 1;

	for (;;){
		heats.push_back();
		if (fscanf(file, "%u%*[,]%u", &heats.back()[0], &heats.back()[1]) != 2){
			heats.pop_back();
			break;
		}
	}
//	if (getc(file) != '\n') sp::raiseError("missing newline\n");
	
	char buffer[128];
	for (;;){
		fgets(buffer, sizeof(buffer), file);
		if (buffer[0] != 'f') break;

		folds.push_back(strtol(buffer+13, nullptr, 10) | ((buffer[11]=='y')<<cordshift));
	}


	auto print = [&heats](){
		int32_t X = 0;
		int32_t Y = 0;
		for (auto &I : heats){
			X = X<I[0] ? I[0] : X;
			Y = Y<I[1] ? I[1] : Y;
		}
		printf("----------------%5i x%4i  ---------------------\n", Y, X);
		for (int32_t i=0; i<=Y; ++i){
			for (int32_t j=0; j<=X; ++j){
				for (auto &I : heats)
					if (j==I[0] && i==I[1]){
						putchar('#');
						goto Skip3;
					}
				putchar(' ');
			Skip3:;
			}	
			putchar('\n');
		}
	};	

	auto I = folds[0];
	{
	//	print();
		
		const int32_t foldpos = I & foldmask;
		auto K = std::begin(heats);

		if ((uint32_t)I >> cordshift){
			for (auto J=K; J!=std::end(heats); ++J){
				int32_t newX = (*J)[0];
				int32_t newY = (*J)[1];

				if (newY > foldpos){
					newY = 2*foldpos - newY;
	//				if (newY < 0) goto Skip0;
					
					for (auto L=std::begin(heats); L!=std::end(heats); ++L)
						if (newX==(*L)[0] && newY==(*L)[1]) goto Skip0;
				}
				(*K)[0] = newX;
				(*K)[1] = newY;
				++K;
			Skip0:;
			}
		} else{
			for (auto J=K; J!=std::end(heats); ++J){
				int32_t newX = (*J)[0];
				int32_t newY = (*J)[1];

				if (newX > foldpos){
					newX = 2*foldpos - newX;
	//				if (newX < 0) goto Skip1;

					for (auto L=std::begin(heats); L!=std::end(heats); ++L)
						if (newX==(*L)[0] && newY==(*L)[1]) goto Skip1;
				}
				(*K)[0] = newX;
				(*K)[1] = newY;
				++K;
			Skip1:;
			}
		}
		heats.resize(K-std::begin(heats));
	}


	print();
	printf("heat points: %lu\n", std::size(heats));

	return 0;
}
