#!/usr/bin/env sh

mkdir -p build

gcc -o build/vector hdvector.c vector.c

gcc -o build/symbolcount symbolcount.c

