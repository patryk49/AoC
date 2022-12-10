#include "SPL/Arrays.hpp"
#include <stdio.h>
#include <array>

#include "Error.hpp"

constexpr int8_t matrices[24][3][3] = {
	{{ 1, 0, 0}, { 0, 1, 0}, { 0, 0, 1}},
	{{ 1, 0, 0}, { 0, 0, 1}, { 0,-1, 0}},
	{{ 1, 0, 0}, { 0,-1, 0}, { 0, 0,-1}},
	{{ 1, 0, 0}, { 0, 0,-1}, { 0, 1, 0}},
	{{ 0, 1, 0}, { 1, 0, 0}, { 0, 0, 1}},
	{{ 0, 1, 0}, { 0, 0, 1}, {-1, 0, 0}},
	{{ 0, 1, 0}, {-1, 0, 0}, { 0, 0,-1}},
	{{ 0, 1, 0}, { 0, 0,-1}, { 1, 0, 0}},
	{{ 0, 0, 1}, { 1, 0, 0}, { 0, 1, 0}},
	{{ 0, 0, 1}, { 0, 1, 0}, {-1, 0, 0}},
	{{ 0, 0, 1}, {-1, 0, 0}, { 0,-1, 0}},
	{{ 0, 0, 1}, { 0,-1, 0}, { 1, 0, 0}},
	{{-1, 0, 0}, { 0, 1, 0}, { 0, 0,-1}},	
	{{-1, 0, 0}, { 0, 0, 1}, { 0, 1, 0}},
	{{-1, 0, 0}, { 0,-1, 0}, { 0, 0, 1}},
	{{-1, 0, 0}, { 0, 0,-1}, { 0,-1, 0}},
	{{ 0,-1, 0}, { 1, 0, 0}, { 0, 0,-1}},
	{{ 0,-1, 0}, { 0, 0, 1}, { 1, 0, 0}},
	{{ 0,-1, 0}, {-1, 0, 0}, { 0, 0, 1}},
	{{ 0,-1, 0}, { 0, 0,-1}, {-1, 0, 0}},
	{{ 0, 0,-1}, { 1, 0, 0}, { 0,-1, 0}},
	{{ 0, 0,-1}, { 0, 1, 0}, { 1, 0, 0}},
	{{ 0, 0,-1}, {-1, 0, 0}, { 0, 1, 0}},
	{{ 0, 0,-1}, { 0,-1, 0}, {-1, 0, 0}}
};

struct Vec3{ int d[3]; };

Vec3 permute(size_t i, int vec[3]) noexcept{
	const int8_t (&mat)[3][3] = matrices[i];
	int res[3];

	res[0] = mat[0][0]*vec[0] + mat[0][1]*vec[1] + mat[0][2]*vec[2];
	res[1] = mat[1][0]*vec[0] + mat[1][1]*vec[1] + mat[1][2]*vec[2];
	res[2] = mat[2][0]*vec[0] + mat[2][1]*vec[1] + mat[2][2]*vec[2];
	return Vec3{res[0], res[1], res[2]};
}



int main(int argc, char **argv){
	FILE *file;
	if (argc == 2){
		if (*argv[1] != 'c') sp::raiseError("wrong argument");
		file = stdin;
	} else{
		file = fopen("inputs/17.dat", "r");
		if (!file) sp::raiseError("file not found");
	}

	sp::DArray<sp::FiniteArray<int [3], 48>> scans;
	
	{
		scans.push_back();
		
		char buffer[64];
		
		fgets(buffer, std::size(buffer), file);
		if (buffer[0]!='-' || buffer[1]!='-') sp::raiseError("wrong starting format\n");

		for (;;){
			fgets(buffer, std::size(buffer), file);
			
			if (buffer[0] == '\n'){
				fgets(buffer, std::size(buffer), file);
				if (buffer[0]!='-' || buffer[1]!='-') break;
				
				scans.push_back();
				continue;
			}

			scans.back().push_back();
			char *ptr = buffer;
			
			scans.back().back()[0] = strtol(ptr, &ptr, 10);
			++ptr;
			scans.back().back()[1] = strtol(ptr, &ptr, 10);
			++ptr;
			scans.back().back()[2] = strtol(ptr, &ptr, 10);
		}	
	}


	for (size_t i=0; i!=24; ++i){
		putchar('\n');
		for (auto &J : scans[0]){
			auto[res] = permute(i, J);
			printf("%6i%6i%6i\n", res[0], res[1], res[2]);
		}
	}



	// positions of beacons are relative to the position of 0'th scanner
	sp::DArray<int [3]> beacons;

	sp::FiniteArray<Vec3, 32> s0;
	sp::FiniteArray<Vec3, 32> s1;

	for (auto I=std::begin(scans); I!=std::end(scans); ++I){
		s0.resize(std::size(*I))	
		for (auto J=I+1; J!=std::end(scans); ++J){
			s1.resize(std::size(*J));

			for (size_t i=0; i!=std::size(matrices); ++i){
				for (size_t j=0; j!=std::size(matrices); ++j){		
			
					int dist[3];
					for (auto II=std::begin(*I); II!=std::end(I); ++II){
						for (size_t ii=0; ii!=std::size(s0); ++ii)
							s0[ii] = permute(i, (*I)[ii]);
						for (auto JJ=std::begin(*J); JJ!=std::end(I); ++JJ){
							for (size_t jj=0; jj!=std::size(s0); ++jj)
								s1[jj] = permute(j, (*J)[jj]);
				

						}
					}
				}
			}
		Next:;
		}
	}




	return 0;
}
