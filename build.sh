#!/usr/bin/env sh

mkdir -p bin

gcc -o bin/vector -Iinc src/hdvector.c src/vector.c

gcc -o bin/symbolcount -Iinc src/symbolcount.c

gcc -o bin/testsuite -Iinc -Itest src/hdvector.c test/test_hdvector.c test/test.c
