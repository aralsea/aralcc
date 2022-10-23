#!/bin/bash
assert() {
  expected="$1"
  input="$2"

  ./aralcc "$input" > tmp.s
  cc -o tmp tmp.s
  ./tmp
  actual="$?"
  if [ "$actual" = "$expected" ]; then
    echo "$input => $actual"
  else
    echo "$input => $expected expected, but got $actual"
    exit 1
  fi
}

assert 0 "0;"
assert 42 "42;"
assert 21 "5+20-4;"
assert 41 " 12 + 34 - 5 ;"
assert 47 "5+6*7;"
assert 15 "5*(9-6);"
assert 4 "(3+5)/2;"
assert 36 "4 * (5 + 4);"
assert 42 "+42;"
assert 21 "(5+20-4);"
assert 17 " -12 + 34 - 5 ;"
assert 47 "5+6*7;"
assert 75 "5*(9-(-6));"
assert 1 "1 == 1;"
assert 1 "1<2;"
assert 0 "3>=19;"
assert 1 "(1+5) != 3;"
assert 0 "0 == 0 == 0;"
assert 0 "0==1;"
assert 1 "42==42;"
assert 1 "0!=1;"
assert 0 "42!=42;"

assert 1 "0<1;"
assert 0 "1<1;"
assert 0 "2<1;"
assert 1 "0<=1;"
assert 1 "1<=1;"
assert 0 "2<=1;"

assert 1 "1>0;"
assert 0 "1>1;"
assert 0 "1>2;"
assert 1 "1>=0;"
assert 1 "1>=1;"
assert 0 "1>=2;"

assert 1 "a = 1; a;"
assert 36 "x = 6; x*x;"
assert 9 "a = 4; b = 5; a+b;"
assert 2 "a=b=1; a+b;"

assert 1 "foo = 1; foo;"
assert 36 "bar = 6; bar*bar;"
assert 9 "foo = 4; bar = 5; foo+bar;"
assert 2 "foo=bar=1; foo+bar;"

assert 1 "foo = 1; return foo;"
assert 36 "bar = 6; return bar*bar;"
assert 9 "foo = 4; bar = 5; return foo+bar;"
assert 2 "foo=bar=1; return foo+bar;"

assert 2 "foo = 1; return 2; foo;"
assert 36 "bar = 6; return bar*bar; return bar;"
assert 9 "foo = 4; bar = 5; return foo+bar; bar = 3;"
assert 2 "foo=bar=1; return foo+bar; x;"

assert 0 "if(0) return 2; return 0;"
assert 2 "if(1+1) return 2; else return 3;"

assert 10 "x=0; while(x<10) x=x+1; return x;"
assert 55 "x=5; sum=5; while(sum<=50) sum = sum + x; return sum;"

assert 45 "sum = 0;for(x=0;x<10;x=x+1) sum = sum + x; return sum;"

assert 1 "{2; return 1;}"
assert 45 "x = 0; sum = 0; while(x<10){sum = sum + x; x = x + 1;} return sum;"
echo OK