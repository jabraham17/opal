
#include "parser/parse_regex.h"
#include "state/DFA.h"

#include <chrono>
#include <fstream>
#include <iostream>
#include <vector>

void toPng(StateList* sl, std::string name, int idx) {
  std::string basename = "imgs/" + name + std::to_string(idx);
  std::string dotName = basename + ".dot";
  std::string pngName = basename + ".png";
  auto g = sl->toGraph("g");
  auto s = g->toString();
  std::ofstream out(dotName);
  out << s << "\n";
  out.close();
  system(std::string("dot -Tpng " + dotName + " -o " + pngName).c_str());
}

int main(int argc, char** argv) {

  Parser p;

  system("mkdir -p imgs");

  for(int i = 1; i < argc; i++) {
    std::string str(argv[i]);

    std::cout << "regex: '" << str << "'\n";

    auto sl = p.parse(str, [](auto msg) { std::cerr << msg << "\n"; });
    if(!sl) {
      std::cerr << "error parsing regex: '" << str << "'\n";
      continue;
    }
    std::cout << "  nfa has " << sl->states().size() << " states\n";
    toPng(&(*sl), "nfa", i);

    sl->prune();

    std::cout << "  pruned-nfa has " << sl->states().size() << " states\n";
    toPng(&(*sl), "pruned-nfa", i);

    auto start = std::chrono::high_resolution_clock::now();
    auto dfa = sl->buildDFA();
    auto stop = std::chrono::high_resolution_clock::now();

    std::cout << "  dfa has " << dfa.states().size() << " states and took "
              << std::chrono::duration_cast<std::chrono::milliseconds>(
                     stop - start)
                     .count()
              << "ms\n";
    // toPng(&dfa, "dfa", i);

    dfa.prune();

    std::cout << "  pruned-dfa has " << dfa.states().size() << " states\n";
    toPng(&dfa, "pruned-dfa", i);

    if(!dfa.isDFA()) {
      std::cerr << "error converting regex: '" << str << "'\n";
      continue;
    }
  }

  return 0;
}
