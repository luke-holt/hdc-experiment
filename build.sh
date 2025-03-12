#!/usr/bin/env sh

set -eux

mkdir -p bin

gcc -o bin/hdc-experiment -Iinc src/hdvector.c src/main.c
gcc -o bin/hdc-experiment-test -Iinc -Itest src/hdvector.c test/test_hdvector.c test/test.c
