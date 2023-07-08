#include "DFA.h"

#include "Instruction.h"

#include <algorithm>
#include <cassert>
#include <sstream>
#include <unordered_map>
#include <unordered_set>

bool State::isDFAEligible() {
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
  auto entry = this->entry();
  auto entryNode = nodes.at(entry);
  g->addEdge(entryDummy, entryNode);

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

bool StateList::isDFA() {
  // all states must be a DFA
  for(const auto& s : this->states()) {
    if(!s->isDFAEligible()) return false;
  }
  // must have an entry as well
  return this->entry() != nullptr;
}
// StateList StateList::buildDFA() {}

/*

power set construction of DFA D from NFA N

states in D is powerset of N states

start state is any state reachable without consuming from the start state of N

accept states of D are any states that have an accept state from N



*/

CompiledRegex StateList::compile() {
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
