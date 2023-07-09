#include "DFA.h"

#include "Instruction.h"

#include <algorithm>
#include <cassert>
#include <queue>
#include <sstream>
#include <unordered_map>
#include <unordered_set>

bool State::isDFAEligible() const {
  // every state we point to is unique and does not have an epsilon trans
  std::unordered_set<State*> states;
  for(auto t : this->transitions()) {
    if(t.isEpsilon()) return false;
    auto [it, inserted] = states.insert(t.toState());
    if(!inserted) return false;
  }

  return true;
}

void StateList::concatWith(StateList& other) {
  // add epsilon transitions from all accepts of first to second's entry (and
  // unmark them as accepts), then add all of second to first

  for(const auto& s : this->states()) {
    if(s->isAccept()) {
      s->addTransition(other.entry());
      s->setAccept(false);
    }
  }

  this->states_.insert(
      this->states_.end(),
      std::make_move_iterator(other.states_.begin()),
      std::make_move_iterator(other.states_.end()));
}

void StateList::unionWith(StateList& other) {
  // add a new entry which has epsilon transitions to both of the old entrys

  auto oldThisEntry = this->entry();
  auto oldOtherEntry = other.entry();

  auto newEntry = std::make_unique<State>();

  newEntry->addTransition(oldThisEntry);
  newEntry->addTransition(oldOtherEntry);

  this->addEntry(std::move(newEntry));

  this->states_.insert(
      this->states_.end(),
      std::make_move_iterator(other.states_.begin()),
      std::make_move_iterator(other.states_.end()));
}

void StateList::kleene() {

  // add a new entry which is also an accept with epsilon to old entry
  // add epsilon from all old accepts to old entry

  auto oldEntry = this->entry();

  for(const auto& s : this->states()) {
    if(s->isAccept()) {
      s->addTransition(oldEntry);
    }
  }

  this->addEntry(std::make_unique<State>(true));
  this->entry()->addTransition(oldEntry);
}

std::unique_ptr<dot::Graph> StateList::toGraph(std::string_view name) {

  auto g = std::make_unique<dot::Graph>(name);
  g->setDirected(true);
  g->setAttribute("rankdir", "LR");

  // add all vertexs
  std::unordered_map<State*, dot::Graph::Vertex*> nodes;
  for(const auto& s : this->states()) {
    auto v = g->addVertex();
    v->setAttribute("label", "\"" + s->name() + "\"");
    if(s->isAccept()) {
      v->setAttribute("shape", "doublecircle");
    }
    nodes.insert_or_assign(s.get(), v);
  }

  auto entryDummy = g->addVertex();
  entryDummy->setAttribute("shape", "point");
  if(auto entry = this->entry()) {
    auto it = nodes.find(entry);
    if(it != nodes.end()) {
      auto entryNode = it->second;
      g->addEdge(entryDummy, entryNode);
    }
  }

  // add all transitions between nodes
  for(const auto& s : this->states()) {
    auto from = nodes.at(s.get());
    for(auto t : s->transitions()) {
      auto to = nodes.at(t.toState());
      auto e = g->addEdge(from, to);
      std::string label = t.isEpsilon() ? "ε" : t.label();
      e->setAttribute("label", "\"" + label + "\"");
    }
  }
  return g;
}

static auto allTransitionsEnteringState(StateList* sl, State* state) {
  std::vector<Transition> trans;
  for(const auto& s : sl->states()) {
    for(auto t : s->transitions()) {
      if(t.toState() == state) {
        trans.push_back(t);
      }
    }
  }
  return trans;
}

void StateList::pruneOnlyEpsilonLeaving() {
  auto it = this->states_.begin();
  while(it != this->states_.end()) {

    auto state = it->get();

    // skip for entry node or accept nodes
    if(this->entry() == state || state->isAccept()) {
      it++;
      continue;
    }

    auto maybeAllEpsilonTrans = state->transitions();
    // are all transitions leaving epsilon?
    bool allEpsilon = std::all_of(
        maybeAllEpsilonTrans.begin(),
        maybeAllEpsilonTrans.end(),
        [](auto t) { return t.isEpsilon(); });

    if(allEpsilon) {
      // for all transitions entering the state, remove it from its `fromState`
      // and add a new one for each `toState` in `maybeAllEpsilonTrans`
      auto transEnteringState = allTransitionsEnteringState(this, state);
      for(auto t : transEnteringState) {

        State* fromState = t.fromState();
        std::string label = t.label();
        fromState->removeTransition(t);
        for(auto toReplace : maybeAllEpsilonTrans) {
          fromState->addTransition(toReplace.toState(), label);
        }
      }

      // remove this state
      it = this->states_.erase(it);
    } else {
      it++;
    }
  }
}

static std::unordered_set<State*> reachableStates(State* s) {
  std::unordered_set<State*> reachable;
  std::unordered_set<State*> explored;
  std::queue<State*> toExplore;
  toExplore.push(s);
  while(!toExplore.empty()) {
    auto next = toExplore.front();
    toExplore.pop();
    reachable.insert(next);
    explored.insert(next);
    for(const auto& t : next->transitions()) {
      if(explored.count(t.toState()) == 0) toExplore.push(t.toState());
    }
  }
  return reachable;
}

#include <fstream>

void StateList::pruneDeadStates() {
  std::unordered_set<State*> pruned;
  // do this until no more
  bool changed = true;
  int i = 0;
  while(changed) {

    changed = false;
    // 1. its a dead state if no transitions leave and its not an accept
    // auto it = this->states_.begin();
    // while(it != this->states_.end()) {

    //   auto state = it->get();
    //   if(this->entry() == state) {
    //     it++;
    //     continue;
    //   }

    //   // if not an accept and no transitions leave, remove it
    //   if(!state->isAccept() && state->transitions().empty()) {
    //     // remove this state
    //     pruned.insert(state);
    //     it = this->states_.erase(it);
    //     changed = true;
    //   } else {
    //     it++;
    //   }
    // }

    // // remove pruned states from transitions
    // for(const auto& s : this->states()) {
    //   for(const auto& t : s->transitions()) {
    //     if(pruned.count(t.toState()) != 0) {
    //       s->removeTransition(t);
    //     }
    //   }
    // }

    // 2. its a dead state if no transitions enter and its not a start
    // walk the tree from the entry and build a set of undead nodes
    auto reachable = reachableStates(this->entry());

    auto it = this->states_.begin();
    while(it != this->states_.end()) {
      auto state = it->get();

      // if not entry and not reachable
      if(this->entry() != state && reachable.count(state) == 0) {
        // remove this state
        it = this->states_.erase(it);
        pruned.insert(state);
        changed = true;
      } else {
        it++;
      }
    }

    // remove pruned states from transitions
    for(const auto& s : this->states()) {
      auto trans = s->transitions();
      for(const auto& t : trans) {
        if(pruned.count(t.toState()) != 0) {
          s->removeTransition(t);
        }
      }
    }

    auto g = toGraph("g");
    auto s = g->toString();
    std::ofstream plain("out" + std::to_string(++i) + ".dot");
    plain << s << "\n";
    plain.close();
    system(std::string(
               "dot -Tpng out" + std::to_string(i) + ".dot -o out" +
               std::to_string(i) + ".png")
               .c_str());
  }
}

bool StateList::isDFA() const {
  // all states must be a DFA
  for(const auto& s : this->states()) {
    if(!s->isDFAEligible()) return false;
  }
  // must have an entry as well
  return this->entry() != nullptr;
}

#include <set>

// have to use set, or define my own hash func for unordered_set
using Set = std::set<State*>;
using PowerSet = std::set<Set>;

static PowerSet
buildPowerset(const std::vector<std::unique_ptr<State>>& states) {
  PowerSet powerset;
  for(const auto& s : states) {
    auto state = s.get();
    for(const auto& set : powerset) {
      auto newSet = set;
      newSet.insert(state);
      powerset.insert(newSet);
    }
    powerset.insert({state});
  }
  Set empty;
  powerset.insert(empty);
  return powerset;
}

static bool containsAcceptState(const Set& set) {
  return std::any_of(set.begin(), set.end(), [](auto s) {
    return s->isAccept();
  });
}

// returns set of all reachable nodes from `from`, including itself
static Set reachableWithoutConsuming(State* from) {
  Set reachable;
  std::queue<State*> toExplore;
  toExplore.push(from);
  while(!toExplore.empty()) {
    auto next = toExplore.front();
    toExplore.pop();
    reachable.insert(next);
    for(const auto& t : next->transitions()) {
      if(t.isEpsilon()) toExplore.push(t.toState());
    }
  }
  return reachable;
}

// walk epsilon transitions from `from` until we run out or find `to`
static bool reachableWithoutConsuming(State* from, State* to) {
  std::queue<State*> toExplore;
  toExplore.push(from);
  while(!toExplore.empty()) {
    auto next = toExplore.front();
    toExplore.pop();
    if(next == to) return true;
    for(const auto& t : next->transitions()) {
      if(t.isEpsilon()) toExplore.push(t.toState());
    }
  }
  return false;
}

static bool allReachableWithoutConsuming(State* from, const Set& set) {
  return std::all_of(set.begin(), set.end(), [from](auto to) {
    return reachableWithoutConsuming(from, to);
  });
}

// linear search here is gonna be slow
static State* findStateInMap(
    const Set& toFind,
    const std::unordered_map<const Set*, State*>& NSetToD) {
  for(const auto& [set, state] : NSetToD) {
    if(toFind == *set) return state;
  }
  return nullptr;
}

/*
power set construction of DFA D from NFA N

states in D is powerset of N states

start state is any state reachable without consuming from the start state of N

accept states of D are any states that have an accept state from N

*/
StateList StateList::buildDFA() const {
  const StateList& N = *this;
  StateList D;

  // build D's states
  // states in D is powerset of N states
  auto powerset = buildPowerset(N.states());
  // contains mapping of D's states to set of N states
  std::unordered_map<State*, const Set*> DToNSet;
  // contains mapping of set of N states to D's states;
  std::unordered_map<const Set*, State*> NSetToD;

  for(const auto& nSet : powerset) {
    State* dState;
    {
      // dStateAlloc is dead after std::move, block prevents programmer
      // mistakes
      std::string label = "";
      std::string sep;
      for(auto s : nSet) {
        label += sep + s->name();
        if(s->name() != "") sep = ",";
      }
      auto dStateAlloc = std::make_unique<State>(label);
      dState = dStateAlloc.get();
      D.add(std::move(dStateAlloc));
    }
    DToNSet.insert_or_assign(dState, &nSet);
    NSetToD.insert_or_assign(&nSet, dState);

    // if any of the states in 'nSet' are an accept state, dState is an accept
    if(containsAcceptState(nSet)) {
      dState->setAccept(true);
    }

    // Determine D's start state: start state is any state reachable without
    // consuming from the start state of N

    // if all of the states in nSet are reachable via an epsilon transition from
    // the start state, this is a start state candidate. if there is no start
    // state, this is our start state. if there is a start state, the larger set
    // of states from N is the new start state
    if(allReachableWithoutConsuming(N.entry(), nSet)) {
      if(!D.entry()) {
        D.setEntry(dState);
      } else {
        auto dEntrySet = DToNSet.at(D.entry());
        if(nSet.size() > dEntrySet->size()) {
          D.setEntry(dState);
        }
      }
    }
  }

  /*
  for dfaState: dfa.states:
    map(char, set(state)) dfaTrans;
    nfaSet = DToNSet[dfaState];
    for nfaState : nfaSet
      for t: nfaState.transitions:
        if ! t.epsilon()
          dfaTrans[t.label].add(t.toState)
          dfaTrans[t.label].add(reachableFrom(t.toState))
  */
  // for each state, build the DFA transfer table, which is all states
  // reachable for a given transition
  // once thats built, we can translate that set into another dfa state, which
  // becomes our toState
  for(const auto& dfaState_ : D.states()) {
    auto dfaState = dfaState_.get();
    std::unordered_map<std::string, Set> dfaTransferTable;
    auto nfaSet = DToNSet.at(dfaState);
    for(const auto& nfaState : *nfaSet) {
      // for each non epsilon transition, add a transition
      // then to the same transition, add all reachable via epsilon
      for(const auto& t : nfaState->transitions()) {
        if(!t.isEpsilon()) {
          auto it = dfaTransferTable.find(t.label());
          if(it == dfaTransferTable.end()) {
            Set empty;
            it = dfaTransferTable.insert(it, {t.label(), empty});
          }
          auto& dfaTransElm = it->second;
          //  returns all reachable without consuming
          for(auto& reachable : reachableWithoutConsuming(t.toState())) {
            dfaTransElm.insert(reachable);
          }
        }
      }
    }
    // convert the set of states in the transfer table into dfaStates
    for(const auto& [label, toNfaSet] : dfaTransferTable) {
      auto toDfaState = findStateInMap(toNfaSet, NSetToD);
      if(toDfaState) dfaState->addTransition(toDfaState, label);
    }
  }

  return D;
}

CompiledRegex StateList::compile() const {
  CompiledRegex cr;

  std::unordered_map<State*, InstructionList*> blocks;
  for(const auto& s : this->states()) {
    auto state = s.get();
    blocks.insert_or_assign(state, cr.getNewBlockForState(state));
  }

  for(const auto& [state, block] : blocks) {

    if(state->isAccept()) {
      // get the first state, which should be a nop
      Instruction* nop = block->head;
      assert(dynamic_cast<NOP*>(nop) != nullptr);
      // add after the nop
      block->addAfter(cr.storeMatch(), nop);
    }

    // add the transitions
    for(auto t : state->transitions()) {
      assert(t.label().size() == 1);
      char c = t.label()[0];
      InstructionList* targetBlock = blocks.at(t.toState());
      Instruction* target = targetBlock->head;
      block->addBack(cr.matchChar(c, target));
    }
  }

  // finish the entry block first
  {
    auto entry = this->entry();
    auto it = blocks.find(entry);
    assert(it != blocks.end());
    auto block = it->second;
    blocks.erase(it);
    cr.finishBlock(block);
  }
  // now finish the rest
  for(const auto& [state, block] : blocks) {
    cr.finishBlock(block);
  }

  return cr;
}

// static std::string wrapName(std::string prefix, std::string name,
//                             std::string suffix) {
//   return prefix + name + suffix;
// }

// std::string toC(std::string nameSuffix, std::string namePrefix) {
//   std::stringstream ss;

//   // function header
//   ss << "long " << wrapName(namePrefix, "match", nameSuffix)
//      << "(const char* input, long n)";
//   ss << " {\n";

//   // variable defs
//   auto counter = wrapName(namePrefix, "i", nameSuffix);
//   ss << "  long " << counter << " = 0;\n";
//   auto longestMatch = wrapName(namePrefix, "longestMatch", nameSuffix);
//   ss << "  long " << longestMatch << " = -1;\n";
//   auto nextChar = wrapName(namePrefix, "next", nameSuffix);
//   ss << "  char " << nextChar << " = '\0';\n";

//   ss << "}";
// }
