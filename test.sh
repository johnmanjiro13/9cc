#!/bin/bash

assert() {
  expected="$1"
  input="$2"

  ./9cc "$input" > tmp.s
  cc -o tmp tmp.s
  ./tmp
  actual="$?"

  if [ "$actual" = "$expected" ]; then
    echo "$input => $actual"
  else
    echo "$input => $expected expected, but go $actual"
    exit 1
  fi
}

assert 0 "return 0;"
assert 42 "return 42;"
assert 21 "return 5+20-4;"
assert 41 "return  12 + 34 - 5 ;"
assert 47 'return 5+6*7;'
assert 8 'return 5+6/2;'
assert 15 'return 5*(9-6);'
assert 4 'return (3+5)/2;'
assert 10 'return -10+20;'
assert 10 'return - -10;'
assert 10 'return - - +10;'

assert 0 'return 0==1;'
assert 1 'return 42==42;'
assert 1 'return 0!=1;'
assert 0 'return 42!=42;'

assert 1 'return 0<1;'
assert 0 'return 1<1;'
assert 0 'return 2<1;'
assert 1 'return 0<=1;'
assert 1 'return 1<=1;'
assert 0 'return 2<=1;'

assert 1 'return 1>0;'
assert 0 'return 1>1;'
assert 0 'return 1>2;'
assert 1 'return 1>=0;'
assert 1 'return 1>=1;'
assert 0 'return 1>=2;'

assert 5 'x=5;return x;'
assert 6 'a=2; b=3; return a*b;'
assert 7 'a=3; b=3+1; return a+b;'
assert 6 'foo=2; bar=3+1; return foo + bar;'

assert 14 'a = 3; b = 5*6-8; return a + b/2;'

assert 1 'a = 1; if (a == 1) return a;'
assert 2 'a = 1; if (a != 1) return 3; else return 2;'

assert 10 "i=0; while(i<10) i=i+1; return i;"

echo OK
