#!/bin/bash

try() {
	expected=$1
	input=$2

	./9cc "$input" > tmp.s
	gcc -o tmp tmp.s
	./tmp
	actual="$?"

	if [ $actual = $expected ]; then
		echo "$input => $actual"
	else
		echo "$expected expected, but got $actual"
		exit 1
	fi
}

try 0 0
try 42 42
try 21 '20+5-4'
try 41 ' 12 + 34 -5 '
try 47 "5+6*7"
try 15 "5*(9-6)"
try 4 "(3+5)/2"
try 5 "-10 +15"
try 0 "5 < 4"
try 0 "1 < 1"
try 1 "-1 < 222"
try 0 "5 <=-4"
try 1 "1 <= 1"
try 1 "-1<=222"
try 0 "-3> 4"
try 0 "1 > 1"
try 1 "-11> -22"
try 0 "5 >=56"
try 1 "0 >=0 "
try 1 "21>= -5"
try 0 "-1==1"
try 1 "5==5"
try 0 "2!=2"
try 1 " -12 !=(2*-4)"

echo OK
