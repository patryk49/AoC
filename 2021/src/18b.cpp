#include "SPL/Arrays.hpp"
#include <stdio.h>
#include <array>

#include "Error.hpp"



void addSnails(sp::DArray<uint8_t> &dest, const sp::DArray<uint8_t> &source) noexcept{
	size_t oldSize = std::size(dest);
	dest.expandBy(std::size(source)+1);
	for (auto I=std::begin(dest)+oldSize; I!=std::begin(dest); --I) *I = *(I-1);
	dest[0] = (uint8_t)-1;
	std::copy(std::begin(source), std::end(source), std::begin(dest)+oldSize+1);

	Repeat:{
		sp::FiniteArray<bool, 4> depths;
		
/*		for (auto I=std::begin(dest);; ++I){
			if (*I == (uint8_t)-1){
				putchar('[');
				depths.push_back(false);
			} else{
				printf("%2u", (uint32_t)*I);
			Rep2:
				if (std::size(depths) == 0) break;
				if (depths.back()){
					depths.pop_back();
					putchar(']');
					goto Rep2;
				} else{
					depths.back() = true;
				}
			}
		}
		putchar('\n');
*/
		for (auto I=std::begin(dest);; ++I){
			if (*I == (uint8_t)-1){
				if (depths.isFull()){
					uint8_t left = I[1];
					uint8_t right = I[2];
					
					std::move(I+3, std::end(dest), I+1);
					dest.shrinkBy(2);
					*I = 0;

					for (auto J=I; J!=std::begin(dest);)
						if (*--J != (uint8_t)-1){
							*J += left;
							break;
						}
					for (auto J=I; ++J!=std::end(dest);)
						if (*J != (uint8_t)-1){
							*J += right;
							break;
						}
					
					goto Repeat;
				}
				depths.push_back(false);
				continue;
			}

		Rep0:
			if (std::size(depths) == 0) break;
			if (depths.back()){
				depths.pop_back();
				goto Rep0;
			} else{
				depths.back() = true;
			}
		}

		for (auto I=std::begin(dest);; ++I){
			if (*I == (uint8_t)-1){
				depths.push_back(false);
				continue;
			}

			if (*I >= 10){
				uint8_t val = *I;
				
				size_t index = I-std::begin(dest);
				dest.expandBy(2);
				auto first = std::begin(dest) + index;
				for (auto J=std::end(dest); --J!=first+2;) *J = *(J-2);
				
				first[0] = (uint8_t)-1;
				first[1] = val >> 1;
				first[2] = (val+1) >> 1;
				goto Repeat;
			}

		Rep1:
			if (std::size(depths) == 0) break;
			if (depths.back()){
				depths.pop_back();
				goto Rep1;
			} else{
				depths.back() = true;
			}
		}
	}
}


size_t getMagnitudeRec(const uint8_t *&iter) noexcept{
	uint8_t val = *iter;
	++iter;

	if (val != (uint8_t)-1) return val;

	size_t left = 3 * getMagnitudeRec(iter);
	return left + 2 * getMagnitudeRec(iter);
}

size_t getMagnitude(const sp::DArray<uint8_t> &arr) noexcept{
	const uint8_t *iter = std::begin(arr);
	return getMagnitudeRec(iter);
}




int main(int argc, char **argv){
	FILE *file;
	if (argc == 2){
		if (*argv[1] != 'c') sp::raiseError("wrong argument");
		file = stdin;
	} else{
		file = fopen("inputs/18.dat", "r");
		if (!file) sp::raiseError("file not found");
	}

	
	sp::DArray<sp::DArray<uint8_t>> numbers;

	for (;;){
		int c = getc(file);
		if (c != '[') break;
	
		numbers.push_back();
		for (; c!='\n'; c=getc(file)){
			if (c==',' || c==']') continue;
			numbers.back().push_back(c=='[' ? (uint8_t)-1 : c-'0');
		}
	}
	puts("input parsing is finished");



	sp::DArray<uint8_t> result;
	size_t max = 0;

	for (auto I=std::begin(numbers); I!=std::end(numbers); ++I)
		for (auto J=std::begin(numbers); J!=std::end(numbers); ++J){
			if (I == J) continue;
			
			result = *I;
			addSnails(result, *J);
			max = std::max(max, getMagnitude(result));
		}

	printf("greatest magnitude: %lu\n", max);

	return 0;
}
