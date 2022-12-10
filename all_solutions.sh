#!/bin/bash

day=1
for solution in $(ls bin); do
	echo day $day: \($(bin/$solution < inputs/$solution.dat \
		| sed -e "s/ans_2:\s*/, /" \
		| sed -e "s/ans_1:\s*//" \
		| sed -e "s/,/|/" \
	)\)
	day=$[day+1]
done
