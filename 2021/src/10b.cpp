#include "SPL/Arrays.hpp"

#include "SPL/Scans.hpp"
#include "Error.hpp"


size_t completionPoints(const sp::DArray<char> &signal) noexcept{
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
			if (stack.back() != '(') return -1;
			stack.pop_back();
			break;
		case ']':
			if (stack.back() != '[') return -1;
			stack.pop_back();
			break;
		case '}':
			if (stack.back() != '{') return -1;
			stack.pop_back();
			break;
		case '>': 
			if (stack.back() != '<') return -1;
			stack.pop_back();
			break;

		default: return (size_t)-1;
		}	
	}

	size_t result = 0;
	while (std::size(stack)){
		result *= 5;
		switch (stack.back()){
		case '<': ++result;
		case '{': ++result;
		case '[': ++result;
		case '(': ++result;
		}
		stack.pop_back();
	}
	return result;
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




	sp::DArray<size_t> scores;
	for (const auto &I : signals){
		const size_t res = completionPoints(I);
		if (res == (size_t)-1) continue;
		scores.push_back(res);
	}
	std::sort(std::begin(scores), std::end(scores));

	printf("points: %lu\n", scores[std::size(scores)/2]);


	return 0;
}
