#!/bin/bash

assert() {
  expected="$1"
  input="$2"

  ./9cc "$input" > tmp.s
  cc -o tmp tmp.s tmp-plus.o
  ./tmp
  actual="$?"

  if [ "$actual" = "$expected" ]; then
    echo "$input => $actual"
  else
    echo "$input => $expected expected, but got $actual"
    exit 1
  fi
}

echo 'int plus(int x, int y) { return x + y; }' | gcc -xc -c -o tmp-plus.o -

assert 0 "main() { return 0; }"
assert 42 "main() { return 42; }"
assert 21 "main() { return 5+20-4; }"
assert 41 "main() { return  12 + 34 - 5 ; }"
assert 47 'main() { return 5+6*7; }'
assert 8 'main() { return 5+6/2; }'
assert 15 'main() { return 5*(9-6); }'
assert 4 'main() { return (3+5)/2; }'
assert 10 'main() { return -10+20; }'
assert 10 'main() { return - -10; }'
assert 10 'main() { return - - +10; }'

assert 0 'main() { return 0==1; }'
assert 1 'main() { return 42==42; }'
assert 1 'main() { return 0!=1; }'
assert 0 'main() { return 42!=42; }'

assert 1 'main() { return 0<1; }'
assert 0 'main() { return 1<1; }'
assert 0 'main() { return 2<1; }'
assert 1 'main() { return 0<=1; }'
assert 1 'main() { return 1<=1; }'
assert 0 'main() { return 2<=1; }'

assert 1 'main() { return 1>0; }'
assert 0 'main() { return 1>1; }'
assert 0 'main() { return 1>2; }'
assert 1 'main() { return 1>=0; }'
assert 1 'main() { return 1>=1; }'
assert 0 'main() { return 1>=2; }'

assert 5 'main() { x=5;return x; }'
assert 6 'main() { a=2; b=3; return a*b; }'
assert 7 'main() { a=3; b=3+1; return a+b; }'
assert 6 'main() { foo=2; bar=3+1; return foo + bar; }'

assert 14 'main() { a = 3; b = 5*6-8; return a + b/2; }'

assert 1 'main() { a = 1; if (a == 1) return a; }'
assert 2 'main() { a = 1; if (a != 1) return 3; else return 2; }'

assert 10 "main() { i=0; while(i<10) i=i+1; return i; }"
assert 10 "main() { for(i=0; i<=10; i=i+1) if (i == 10) return i; }"

assert 3 'main() { {1; {2;} return 3;} }'
assert 55 'main() { i=0; j=0; while(i<=10) {j=i+j; i=i+1;} return j; }'

assert 5 'main() { return plus(2, 3); }'
assert 6 'main() {  fuga = 1;  return bar(fuga, 2, 3);} bar(a, b, c) {  return a + b + c;}'

echo OK
