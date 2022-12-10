#!/bin/bash

g++ src/$1.cpp -o bin/$1 -O3 -std=c++17 -Iinclude
