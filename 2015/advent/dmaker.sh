#!/bin/bash

g++ src/$1.cpp -o bin/$1 -g -std=c++17 -Iinclude
