#!/bin/bash

g++ src/$1.cpp -o bin/$1 -g -O0 \
	-std=c++20 -Iinclude -fno-exceptions \
	-Wall -Wextra -Wno-attributes -Wno-implicit-fallthrough
