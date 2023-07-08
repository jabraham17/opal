
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
  // //  (ba|c*)d
  // std::string regex = "(((b).(a))|((c)*)).(d)";
  // // std::string regex = "(a)|(_)";
  // // std::string regex = "(b).((a)*)";
  // std::string regex = "(a)*";
  // std::string regex = "_";
  // std::string regex = "(a).(b)";
  std::string regex = "(((a).(b))|(a))*"; // (ab|a)*
  auto newSl = p.parse(regex, [](auto msg) { std::cerr << msg << "\n"; });

  // StateList sl;
  // {
  //   auto q1 = std::make_unique<State>("q1", true);
  //   auto q2 = std::make_unique<State>("q2");
  //   auto q3 = std::make_unique<State>("q3");
  //   q1->addTransition(q3.get());
  //   q1->addTransition(q2.get(), 'b');
  //   q2->addTransition(q2.get(), 'a');
  //   q2->addTransition(q3.get(), 'a');
  //   q2->addTransition(q3.get(), 'b');
  //   q3->addTransition(q1.get(), 'a');
  //   sl.addEntry(std::move(q1));
  //   sl.add(std::move(q2));
  //   sl.add(std::move(q3));
  // }
  // StateList* newSl = &sl;

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

  auto dfa = newSl->buildDFA();
  std::cout << "nfa with " << newSl->states().size()
            << " states became a dfa with " << dfa.states().size()
            << " states\n";
  dfa.prune();
  std::cout << "after pruning: " << dfa.states().size() << "\n";

  auto g3 = dfa.toGraph("g3");
  auto s3 = g3->toString();
  std::ofstream dfa_g("dfa.dot");
  dfa_g << s3 << "\n";
  dfa_g.close();
  system("dot -Tpng dfa.dot -o dfa.png");

  if(dfa.isDFA()) {

    system("mkdir -p testing/bin");

    auto cr = dfa.compile();
    std::ofstream outc("testing/bin/matcher.c");
    outc << cr.toC("match") << "\n";
    outc.close();

    system("clang -c testing/bin/matcher.c -o testing/bin/matcher_c.o");

    std::ofstream outa("testing/bin/matcher.asm");
    outa << cr.toNasm("match") << "\n";
    outa.close();

    system("nasm -felf64 testing/bin/matcher.asm -o testing/bin/matcher_asm.o");

    system("nasm -felf64 testing/inAsm.asm -o testing/bin/inAsm.o");
    system("clang testing/test.c testing/bin/inAsm.o testing/bin/matcher_c.o "
           "-o match_c.out");
    system("clang testing/test.c testing/bin/inAsm.o testing/bin/matcher_asm.o "
           "-o match_asm.out");
  } else {
    std::cout << "regex: '" << regex << "' is not a DFA yet\n";
  }

  return 0;
}
