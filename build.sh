#!/usr/bin/sh
set -e
set -x
clang++ *.cpp -std=c++17 -Wall -Wextra -o opal.out $@
