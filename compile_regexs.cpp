
#include "DFA.h"

#include <fstream>
#include <iostream>
#include <vector>

#include "parse_regex.h"

int main(int argc, char** argv) {

  Parser p;
  std::vector<std::pair<std::string, CompiledRegex>> patterns;

  for(int i = 1; i < argc; i++) {

    std::string str(argv[i]);
    auto sl = p.parse(str, [](auto msg) { std::cerr << msg << "\n"; });
    if(!sl) {
      std::cerr << "error parsing regex: '" << str << "'\n";
      continue;
    }
    sl->prune();
    auto dfa = sl->buildDFA();
    dfa.prune();

    if(!dfa.isDFA()) {
      std::cerr << "error converting regex: '" << str << "'\n";
      continue;
    }

    auto cr = dfa.compile();
    patterns.push_back({str, std::move(cr)});
  }

  system("mkdir -p bin");
  std::ofstream outAsm("bin/matchers.asm");
  std::ofstream outDefs("bin/defs.c");




  for(auto it = patterns.begin(); it != patterns.end(); it++) {
    auto idx = std::distance(patterns.begin(), it);
    std::string name = "match" + std::to_string(idx);
    outAsm << it->second.toNasm(name) << "\n";
    outDefs << it->second.toHeader(name) << "\n";
  }



    outDefs << "typedef long (*matchFunc)(const char*, long);\n";

  outDefs << "long nPatterns = " << std::to_string(patterns.size()) << ";\n";
  outDefs << "matchFunc funcs[] = {\n";
  for(auto it = patterns.begin(); it != patterns.end(); it++) {
    auto idx = std::distance(patterns.begin(), it);
    outDefs << "match" << std::to_string(idx) << ",\n";
  }
  outDefs << "};\n";
  outDefs << "const char* patterns[] = {\n";
  for(auto it = patterns.begin(); it != patterns.end(); it++) {
    outDefs << "\"" << it->first << "\",\n";
  }
  outDefs << "};\n";

  outAsm.close();
  outDefs.close();

  system("nasm -felf64 bin/matchers.asm -o bin/matchers.o");
  system("clang template/main.c bin/defs.c bin/matchers.o -o a.out");

  return 0;
}
