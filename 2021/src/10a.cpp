#include "SPL/Arrays.hpp"

#include "SPL/Scans.hpp"
#include "Error.hpp"


size_t errorPoints(const sp::DArray<char> &signal) noexcept{
	sp::DArray<char> stack;
	for (size_t i=0; i!=std::size(signal); ++i){
		switch (signal[i]){
		case '(':
		case '[':
		case '{':
		case '<':
			stack.push_back(signal[i]);
			break;

		case ')':
			if (stack.back() != '(') return 3;
			stack.pop_back();
			break;
		case ']':
			if (stack.back() != '[') return 57;
			stack.pop_back();
			break;
		case '}':
			if (stack.back() != '{') return 1197;
			stack.pop_back();
			break;
		case '>': 
			if (stack.back() != '<') return 25137;
			stack.pop_back();
			break;
		
		default: return (size_t)-1;
		}	
	}
	return 0;
}





int main(int argc, char **argv){
	FILE *file;
	if (argc == 2){
		if (*argv[1] != 'c') sp::raiseError("wrong argument");
		file = stdin;
	} else{
		file = fopen("inputs/10.dat", "r");
		if (!file) sp::raiseError("file not found");
	}


	sp::DArray<sp::DArray<char>> signals;
	signals.push_back();

	for (;;){
		const char c = getc(file);
		if (c == '\n'){
			signals.push_back();
			continue;
		}
		for (auto I : "(){}[]<>") if (c == I) goto Continue;
		break;
	Continue:
		signals.back().push_back(c);
	}




	size_t points = 0;
	for (const auto &I : signals) points += errorPoints(I);

	printf("points: %lu\n", points);


	return 0;
}
