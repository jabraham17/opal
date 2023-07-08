
#ifndef DFA_H_
#define DFA_H_
#include "Dot.h"
#include "Instruction.h"

#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <utility>
#include <vector>

class State;
class Transition {
private:
  State* fromState_;
  State* toState_;
  std::string label_;

public:
  Transition(State* fromState, State* toState, std::string label = "")
      : fromState_(fromState), toState_(toState), label_(label) {}

  State* fromState() { return fromState_; }
  State* toState() { return toState_; }
  std::string label() { return label_; }
  bool isEpsilon() { return label_.empty(); }

  friend bool operator==(const Transition& lhs, const Transition& rhs) {
    return lhs.fromState_ == rhs.fromState_ && lhs.toState_ == rhs.toState_ &&
           lhs.label_ == rhs.label_;
  }
};

class State {
private:
  std::string name_;
  std::vector<Transition> transitions_;

  bool isAccept_;

public:
  State(bool isAccept = false) : State("", isAccept) {}
  State(std::string name, bool isAccept = false)
      : name_(name), isAccept_(isAccept) {}
  ~State() = default;
  State(const State& other) = delete;
  State(State&& other) noexcept = default;
  State& operator=(const State& other) = delete;
  State& operator=(State&& other) noexcept = default;

  void addTransition(State* to, std::string label = "") {
    Transition t(this, to, label);
    transitions_.push_back(t);
  }
  void addTransition(State* to, char c) {
    addTransition(to, std::string(1, c));
  }
  void removeTransition(Transition t) {
    auto it =
        std::find(this->transitions_.begin(), this->transitions_.end(), t);
    if(it != this->transitions_.end()) {
      this->transitions_.erase(it);
    }
  }
  std::string name() { return name_; }
  std::vector<Transition> transitions() { return transitions_; }

  bool isAccept() { return isAccept_; }
  void setAccept(bool e = true) { isAccept_ = e; }

  // each state is unique and no epsilon
  bool isDFAEligible();
};

// class AssembledRegex {
// public:
//   struct Instruction {
//     std::string inst;
//     bool isLabeled;
//   };

// private:
//   std::vector<Instruction> instructions_;

// public:
//   void addInstruction(std::string inst, bool isLabeled = false);

//   std::string toString();
// };

// class JITRegex {
// public:
//   using MatchFunction = long (*)(char *, long);

// private:
//   void *memory;
//   size_t memSize;
//   MatchFunction funcPtr;

// public:
// JITRegex(const AssembledRegex& assembled) {
//   // TODO: mmap code to map assembled code
// }
//   MatchFunction function() {return funcPtr;}
//   long operator()(char *input, long n) {
//     if (funcPtr)
//       return funcPtr(input, n);
//     else
//       return -1;
//   }
// };

class StateList {
private:
  std::vector<std::unique_ptr<State>> states_;
  State* entry_;

public:
  StateList() = default;
  ~StateList() = default;
  StateList(const StateList& other) = delete;
  // StateList(const StateList& other) {
  //   for(auto& o: other.states_) {
  //     auto n = std::make_unique<State>(*o);
  //     if(o.get() == other.entry()) {
  //       this->entry_ = n.get();
  //     }
  //     this->states_.push_back(std::move(n));
  //   }
  // }
  StateList(StateList&& other) noexcept = default;
  StateList& operator=(const StateList& other) = delete;
  StateList& operator=(StateList&& other) noexcept = default;

  // void doOnAllTransitions(std::function<void(State* s, Transition t)> func) {
  //   for(auto& s: this->states_) {
  //     for(auto t: s->transitions()) {
  //       func(s.get(), t);
  //     }
  //   }
  // }

  const std::vector<std::unique_ptr<State>>& states() {
    return const_cast<const StateList*>(this)->states();
  }
  const std::vector<std::unique_ptr<State>>& states() const { return states_; }
  void add(std::unique_ptr<State> s) { states_.push_back(std::move(s)); }
  void addEntry(std::unique_ptr<State> s) {
    setEntry(s.get());
    add(std::move(s));
  }
  void setEntry(State* s) { this->entry_ = s; }
  State* entry() { return const_cast<const StateList*>(this)->entry(); }
  State* entry() const {
    if(states_.empty()) return nullptr;
    else if(entry_ != nullptr) return entry_;
    else return states_[0].get();
  }
  void concatWith(StateList& other);
  void unionWith(StateList& other);
  void kleene();
  std::unique_ptr<dot::Graph> toGraph(std::string_view);

  void prune() { pruneOnlyEpsilonLeaving(); }

  bool isDFA();
  StateList buildDFA();

  CompiledRegex compile();

  // std::string toC(std::string nameSuffix = "", std::string namePrefix="rr");
  // std::string toAsm();

private:
  // if all nodes leaving node are epsilon, we can remove this node
  void pruneOnlyEpsilonLeaving();
};

// class NFA {
// private:
//   StateList states;

// private:
//   NFA();

// public:
//   static std::unique_ptr<NFA> build(StateList states);
//   static bool isNFA(StateList states);
// };

// class DFA {
//   private:
//   StateList states;

//   private:
//   DFA();
// };

#endif
