#include "SPL/Arrays.hpp"

#include "SPL/Scans.hpp"
#include "Error.hpp"


bool isAlpha(const char c) noexcept{ return (c>='a' && c<='z') || (c>='A' && c<='Z'); }


void copySelected(
	sp::DArray<uint8_t [2]> &dest, const sp::DArray<uint8_t [2]> &src, const size_t index
) noexcept{
	for (auto &I : src) if (I[0]==index || I[1]==index){
		dest.push_back();
		dest.back()[0] = I[0];
		dest.back()[1] = I[1];
	}
};


void moveSelected(
	sp::DArray<uint8_t [2]> &dest, sp::DArray<uint8_t [2]> &src, const size_t index
) noexcept{
	auto J = std::begin(src);
	for (auto I=J; I!=std::end(src); ++I){
		if ((*I)[0]==index || (*I)[1]==index){
			dest.push_back();
			dest.back()[0] = (*I)[0];
			dest.back()[1] = (*I)[1];
		} else{
			(*J)[0] = (*I)[0];
			(*J)[1] = (*I)[1];
			++J;
		}
	}
	src.endPtr = J;
};



size_t ways(const sp::DArray<uint8_t [2]> &adjList) noexcept{
	bool seen[128];
	memset(seen, 0, sizeof(seen));

	sp::DArray<uint8_t> stack;
	stack.push_back(0);

	size_t ways = 0;

	while (std::size(stack)){
		stack.pop_back();
		const uint8_t index = stack.back();
		
		if (index == (uint8_t)-1){
			stack.pop_back();
			seen[*std::end(stack)] = false;
			continue;
		}

		const bool isSmall = index < (1<<7);

		if (isSmall){
			if (seen[index]) continue;
			seen[index] = true;
			stack.push_back(index);
			stack.push_back(-1);
		}

		for (auto I : adjList)
			if (I[0]==index || I[1]==index){
				const size_t offset = I[0] == index;
				if (I[offset] == 1){
					++ways;
					continue;
				}
				stack.push_back(I[offset]);
			}
	}
	
	return ways;
}


size_t getWaysRec(
	const sp::DArray<uint8_t [2]> &adjList, const uint8_t index, uint8_t (&seen)[128], bool flag
) noexcept{
	const bool isLittle = index < (1<<7);
	
	bool setSeen = false;
	if (isLittle){
		if (flag){
			if (seen[index]) return 0;
		} else{
			flag = seen[index];
		}
		
		setSeen = true;
		++seen[index];
	}

	size_t ways = 0;

	for (auto &I : adjList)
		if (I[0]==index || I[1]==index){
			const uint8_t next = I[I[0]==index];
			if (next == 0) continue;
			
			ways += next==1 ? 1 : getWaysRec(adjList, next, seen, flag);
		}
	
	if (isLittle) seen[index] -= setSeen;

	return ways;
}


size_t getWays(const sp::DArray<uint8_t [2]> &adjList) noexcept{
	uint8_t seen[128];
	memset(seen, 0, sizeof(seen));
	return getWaysRec(adjList, 0, seen, false);
}


int main(int argc, char **argv){
	FILE *file;
	if (argc == 2){
		if (*argv[1] != 'c') sp::raiseError("wrong argument");
		file = stdin;
	} else{
		file = fopen("inputs/12.dat", "r");
		if (!file) sp::raiseError("file not found");
	}


	sp::DArray<uint8_t [2]> adjList;

	{
		sp::DArray<sp::FiniteArray<char, 8>> map;
		const char *const st = "start";
		const char *const en = "end";
		map.emplace_back(st, st+5);
		map.emplace_back(en, en+3);

		sp::FiniteArray<char, 32> buffer;
		for (;;){
			fgets(std::begin(buffer), buffer.capacity(), file);
			if (!isAlpha(buffer.front())) break;

			for (const char *I=std::begin(buffer); isAlpha(*I); ++I) buffer.push_back();
			
			size_t index = 0;
			for (auto &I : map){
				if (buffer == I) goto Found0;
				++index;
			}
			map.push_back(buffer);
		Found0:
			adjList.push_back();
			adjList.back()[0] = index | ((map[index][0] < 'a') << 7);

			std::move(std::end(buffer)+1, std::begin(buffer)+buffer.capacity(), std::begin(buffer));
			buffer.resize(0);
			for (const char *I=std::begin(buffer); isAlpha(*I); ++I) buffer.push_back();
		
			index = 0;
			for (auto &I : map){
				if (buffer == I) goto Found1;
				++index;
			}
			map.push_back(buffer);
		Found1:
			adjList.back()[1] = index | ((map[index][0] < 'a') << 7);
			
			if (*buffer.end() != '\n') break;
			buffer.resize(0);
		}
	}



//	printf("ways iteratively:  %lu\n", ways(adjList));
	printf("ways reccursively: %lu\n", getWays(adjList));


	return 0;
}
