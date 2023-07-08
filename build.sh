#!/usr/bin/sh
set -e
set -x
SRC='DFA.cpp Dot.cpp Instruction.cpp parse_regex.cpp utils.cpp'
clang++ main.cpp $SRC -std=c++17 -Wall -Wextra -o test.out $@
clang++ compile_regexs.cpp $SRC -std=c++17 -Wall -Wextra -o opal.out $@
clang++ debugger.cpp $SRC -std=c++17 -Wall -Wextra -o debugger.out $@
