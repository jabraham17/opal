
#include "DFA.h"

#include <fstream>
#include <iostream>

#include "parse_regex.h"

int main() {

  // StateList sl;

  // // a
  // {
  //   auto entry = std::make_unique<State>();
  //   auto accept = std::make_unique<State>(true);
  //   Transition t(accept.get(), "a");
  //   entry->addTransition(t);
  //   sl.addEntry(std::move(entry));
  //   sl.add(std::move(accept));
  // }

  // // b
  // StateList sl_b;
  // {
  //   auto entry = std::make_unique<State>();
  //   auto accept = std::make_unique<State>(true);
  //   Transition t(accept.get(), "b");
  //   entry->addTransition(t);
  //   sl_b.addEntry(std::move(entry));
  //   sl_b.add(std::move(accept));
  // }

  // // ab
  // sl.concatWith(sl_b);

  // // ab|a
  // {
  //   StateList sl_a;
  //   auto entry = std::make_unique<State>();
  //   auto accept = std::make_unique<State>(true);
  //   Transition t(accept.get(), "a");
  //   entry->addTransition(t);
  //   sl_a.addEntry(std::move(entry));
  //   sl_a.add(std::move(accept));
  //   sl.unionWith(sl_a);
  // }

  // sl.kleene();

  // auto g = sl.toGraph("g");
  // auto s = g->toString();
  // std::cout << s << "\n";

  Parser p;
  //  (ba|c*)d
  // auto newSl = p.parse("(((b).(a))|((c)*)).(d)",
  //                      [](auto msg) { std::cerr << msg << "\n"; });
  auto newSl = p.parse("(a).(b)");

  if(!newSl) {
    std::cerr << "error parsing regex\n";
    return 1;
  }

  auto g = newSl->toGraph("g");
  auto s = g->toString();
  std::ofstream plain("out.dot");
  plain << s << "\n";
  plain.close();
  system("dot -Tpng out.dot -o out.png");

  newSl->prune();

  auto g2 = newSl->toGraph("g2");
  auto s2 = g2->toString();
  std::ofstream pruned("pruned.dot");
  pruned << s2 << "\n";
  pruned.close();
  system("dot -Tpng pruned.dot -o pruned.png");

  system("mkdir -p testing/bin");

  auto cr = newSl->compile();
  std::ofstream outc("testing/bin/matcher.c");
  outc << cr.toC("match") << "\n";
  outc.close();

  system("clang -c testing/bin/matcher.c -o testing/bin/matcher_c.o");

    std::ofstream outa("testing/bin/matcher.asm");
  outa << cr.toNasm("match") << "\n";
  outa.close();

  system("nasm -felf64 testing/bin/matcher.asm -o testing/bin/matcher_asm.o");


  system("nasm -felf64 testing/inAsm.asm -o testing/bin/inAsm.o");
  system("clang testing/test.c testing/bin/inAsm.o testing/bin/matcher_c.o -o match_c");
  system("clang testing/test.c testing/bin/inAsm.o testing/bin/matcher_asm.o -o match_asm");

  return 0;
}
