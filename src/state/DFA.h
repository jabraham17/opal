#ifndef OPAL_STATE_DFA_H_
#define OPAL_STATE_DFA_H_

#ifndef DFA_H_
#define DFA_H_
#include "codegen/Instruction.h"
#include "dot/Dot.h"

#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <type_traits>
#include <typeinfo>
#include <utility>
#include <vector>

#define CONST_THIS                                                             \
  const_cast<std::add_pointer_t<                                               \
      std::add_const_t<std::remove_pointer_t<decltype(this)>>>>(this)

// clang-format off
#define CONST_MEMBER_FUNC_BODY(retType, name, body)                            \
  retType name() const body \
  retType name() { return CONST_THIS->name(); }
// clang-format on

#define CONST_MEMBER_FUNC(retType, name)                                       \
  CONST_MEMBER_FUNC_BODY(retType, name, ;)

#define CONST_MEMBER_FUNC_GETTER_TYPED(retType, name, val)                     \
  CONST_MEMBER_FUNC_BODY(retType, name, { return val; })

#define CONST_MEMBER_FUNC_GETTER(name, val)                                    \
  CONST_MEMBER_FUNC_GETTER_TYPED(auto, name, val)

#define CONST_MEMBER_FUNC_NO_RET(name)                                         \
  void name() const;                                                           \
  void name() { CONST_THIS->name(); }

class State;
class Transition {
private:
  State* fromState_;
  State* toState_;
  std::string label_;

public:
  Transition(State* fromState, State* toState, std::string label = "")
      : fromState_(fromState), toState_(toState), label_(label) {}

  CONST_MEMBER_FUNC_GETTER(fromState, fromState_);
  CONST_MEMBER_FUNC_GETTER(toState, toState_);
  CONST_MEMBER_FUNC_GETTER(label, label_);
  CONST_MEMBER_FUNC_GETTER(isEpsilon, label_.empty());

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
  State(const std::string& name, bool isAccept = false)
      : name_(name), isAccept_(isAccept) {}

  //  this nastiness is required because string literals decay to char* and
  //  char* implicitly converts to bool
  template <typename Y, typename T = std::enable_if_t<std::is_same<Y, bool>{}>>
  State(Y isAccept) : State("", isAccept) {}
  State() : State(false) {}

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

  CONST_MEMBER_FUNC_GETTER(name, name_);
  CONST_MEMBER_FUNC_GETTER_TYPED(const auto&, transitions, transitions_);
  CONST_MEMBER_FUNC_GETTER(isAccept, isAccept_);

  void setAccept(bool e = true) { isAccept_ = e; }

  // each state is unique and no epsilon
  CONST_MEMBER_FUNC(bool, isDFAEligible);
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
  State* entry_ = nullptr;

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

  CONST_MEMBER_FUNC_GETTER_TYPED(const auto&, states, states_);

  void add(std::unique_ptr<State> s) { states_.push_back(std::move(s)); }
  void addEntry(std::unique_ptr<State> s) {
    setEntry(s.get());
    add(std::move(s));
  }
  void setEntry(State* s) { this->entry_ = s; }

  // CONST_MEMBER_FUNC_BODY(State*, entry, {
  //   if(states_.empty()) return nullptr;
  //   else if(entry_ != nullptr) return entry_;
  //   else return nullptr;
  // });
  CONST_MEMBER_FUNC_GETTER(entry, entry_);

  void concatWith(StateList& other);
  void unionWith(StateList& other);
  void kleene();
  std::unique_ptr<dot::Graph> toGraph(std::string_view);

  void prune() {
    pruneOnlyEpsilonLeaving();
    pruneDeadStates();
    // if no transitions leave and its not an accept, prune
    // if no transitions enter and its not an accept, prune
  }

  CONST_MEMBER_FUNC(bool, isDFA);
  CONST_MEMBER_FUNC(StateList, buildDFA);

  CONST_MEMBER_FUNC(CompiledRegex, compile);

  // std::string toC(std::string nameSuffix = "", std::string namePrefix="rr");
  // std::string toAsm();

private:
  // if all nodes leaving node are epsilon, we can remove this node
  void pruneOnlyEpsilonLeaving();
  // its a dead state if no transitions leave and its not an accept
  // its a dead state if no transitions enter and its not a start
  void pruneDeadStates();
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
#endif
