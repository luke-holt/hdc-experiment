#!/usr/bin/env sh

mkdir -p bin

gcc -o bin/vector hdvector.c vector.c

gcc -o bin/symbolcount symbolcount.c

