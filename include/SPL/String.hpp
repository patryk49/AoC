#include "Utils.hpp"
#include "Arrays.hpp"

#pragma once




template<class T, class A>
Array<Span<T>> split(Span<T> string, Span<const T> pattern, A &al) noexcept{
	Array<Span<T>> result = {};
	push_value(result, Span<T>{beg(string), 0}, al);
	
	size_t prev_match = 0;
	for (int i=0; i<(int)(string.size-pattern.size+1);){
		for (size_t j=0; j!=pattern.size; j+=1)
			if (string[i+j] != pattern[j]){
				i += 1;
				goto Continue;
			}
		result[result.size-1].size = i - prev_match;
		prev_match = i + pattern.size;
		push_value(result, Span<T>{beg(string)+prev_match, 0}, al);
		i += pattern.size;
	Continue:;
	}

	result[result.size-1].size = string.size - prev_match;
	return result;
}

template<size_t S, class A>
Array<Span<char>> split(Span<char> string, const char (&pattern)[S], A &al) noexcept{
	return split(string, Span<const char>{(const char *)pattern, S-1}, al);
}

template<class T, class A>
Array<Span<T>> split(Span<T> string, T token, A &al) noexcept{
	Array<Span<T>> result = {};
	push_value(result, Span<T>{beg(string), 0}, al);
	
	size_t prev_match = 0;
	for (size_t i=0; i!=string.size; i+=1){
		if (string[i] == token){
			result[result.size-1].size = i - prev_match;
			prev_match = i + 1;
			push_value(result, Span<T>{beg(string)+prev_match, 0}, al);
		}
	}

	result[result.size-1].size = string.size - prev_match;
	return result;
}

