#ifndef INSTRUCTION_H_
#define INSTRUCTION_H_

#include <array>
#include <cassert>
#include <memory>
#include <string>
#include <vector>

enum class Types { CHAR, CHAR_STAR, CONST_CHAR_STAR, LONG, INT };
std::string toString(Types t);

enum class X86Register { NONE, DI, SI, D, A, C, R8, R9, R10, R11 };
std::string toString(X86Register reg, Types t = Types::LONG);

extern std::array<X86Register, 6> argumentRegisters;
extern X86Register returnRegister;
extern std::array<X86Register, 2> tmpRegisters;

// in c, a variable
// in asm, a register
struct Variable {
  Types type;
  unsigned long long initialValue = 0;
  std::string name;
  X86Register reg;

private:
  bool isParameter_ = false;
  bool isImmediate_ = false;
  bool hasInitialValue_ = false;

public:
  bool hasInitialValue() { return hasInitialValue_; }
  bool hasName() { return !isImmediate_; }
  bool isParameter() { return isParameter_; }
  bool isLocal() { return !isParameter_ && !isImmediate_; }
  bool isImmediate() { return isImmediate_; }

  std::string getInitialValue(bool forAsm = false) {
    assert(hasInitialValue());
    if(!forAsm)
      return "(" + toString(type) + ")" + std::to_string(initialValue) + "U";
    else return std::to_string(initialValue);
  }

  std::string getLValue(bool forAsm = false) {
    assert(!isImmediate());
    return !forAsm ? name : getReg();
  }
  std::string getRValue(bool forAsm = false) {
    if(isImmediate()) return getInitialValue(forAsm);
    else return !forAsm ? name : getReg();
  }

  std::string getReg() { return toString(reg, type); }

  static Variable buildParameter(
      Types type,
      std::string name,
      X86Register reg = X86Register::NONE) {
    Variable v;
    v.type = type;
    v.name = name;
    v.isParameter_ = true;
    v.hasInitialValue_ = false;
    v.reg = reg;
    return v;
  }
  static Variable buildLocal(
      Types type,
      std::string name,
      unsigned long long initialValue,
      X86Register reg = X86Register::NONE) {
    Variable v;
    v.type = type;
    v.name = name;
    v.initialValue = initialValue;
    v.hasInitialValue_ = true;
    v.reg = reg;
    return v;
  }
  static Variable buildLocal(
      Types type,
      std::string name,
      X86Register reg = X86Register::NONE) {
    Variable v;
    v.type = type;
    v.name = name;
    v.hasInitialValue_ = false;
    v.reg = reg;
    return v;
  }
  static Variable* buildImmediate(Types type, unsigned long long value) {
    Variable* v = new Variable;
    v->type = type;
    v->initialValue = value;
    v->isImmediate_ = true;
    v->hasInitialValue_ = true;
    return v;
  }
};
struct Instruction {
  Instruction* prev;
  Instruction* next;
  virtual ~Instruction() = default;
  virtual std::vector<unsigned char> toBytes() { return {}; }
  virtual std::string toC() { return ""; }
  virtual std::string toNasm() { return ""; }
  virtual std::string getCLabel() {
    return "state_" + std::to_string(intptr_t(this));
  }
  virtual std::string getNasmLabel() { return "." + getCLabel(); }
};
struct NOP : public Instruction {
  std::vector<unsigned char> toBytes() override { return {}; }
  std::string toC() override { return ""; }
  std::string toNasm() override { return this->getNasmLabel() + ":"; }
};

std::string loadExtForType(Types destType, Types loadType);
std::string loadSizeForType(Types loadType);

struct Load : public Instruction {
  Types loadType;

  Variable* dest;
  Variable* base;
  Variable* offset;
  std::vector<unsigned char> toBytes() override { return {}; }
  std::string toC() override { return ""; }
  std::string toNasm() override {
    std::string ext = loadExtForType(dest->type, loadType);
    std::string size = loadSizeForType(loadType);

    std::string ret = "mov " + ext + " " + dest->getLValue(true) + "," + size +
                      " [" + base->getRValue(true) + " + " +
                      offset->getRValue(true) + "]";
    return ret;
  }
};
struct Add : public Instruction {
  Variable* dest;
  Variable* op1;
  std::vector<unsigned char> toBytes() override { return {}; }
  std::string toC() override { return ""; }
  std::string toNasm() override {
    std::string ret =
        "add " + dest->getLValue(true) + ", " + op1->getRValue(true);
    return ret;
  }
};
enum class ConditionCode { EQ, NEQ, GT, GTEQ, LT, LTEQ };
std::string toString(ConditionCode cc, bool isAsm = false);
struct Jump : public Instruction {
  Instruction* target;
  std::vector<unsigned char> toBytes() override { return {}; }
  std::string toC() override { return ""; }
  std::string toNasm() override {
    std::string ret = "jmp " + target->getNasmLabel();
    return ret;
  }
};
struct ConditionalJump : public Instruction {
  Variable* lhs;
  Variable* rhs;
  ConditionCode cc;
  Instruction* target;
  std::vector<unsigned char> toBytes() override { return {}; }
  std::string toC() override { return ""; }
  std::string toNasm() override {
    std::string ret = "cmp " + lhs->getRValue(true) + ", " +
                      rhs->getRValue(true) + "\n" + "j" + toString(cc, true) +
                      " " + target->getNasmLabel();
    return ret;
  }
};
struct Copy : public Instruction {
  Variable* dest;
  Variable* source;
  std::vector<unsigned char> toBytes() override { return {}; }
  std::string toC() override { return ""; }
  std::string toNasm() override {
    std::string ret =
        "mov " + dest->getLValue(true) + ", " + source->getRValue(true);
    return ret;
  }
};
struct Return : public Instruction {
  Variable* value;
  std::vector<unsigned char> toBytes() override { return {}; }
  std::string toC() override { return ""; }
  std::string toNasm() override {
    std::string ret = this->getNasmLabel() + ":\n" + "ret";
    return ret;
  }
};

struct InstructionList;
class InstructionListIterator {
public:
  using iterator_category = std::bidirectional_iterator_tag;
  using value_type = Instruction*;
  using difference_type = std::ptrdiff_t;
  using pointer = value_type*;
  using reference = value_type&;

private:
  InstructionList* list;
  Instruction* curr_;

public:
  InstructionListIterator() = default;
  InstructionListIterator(InstructionList* list);
  InstructionListIterator(InstructionList* list, Instruction* curr)
      : list(list), curr_(curr) {}
  ~InstructionListIterator() = default;

  InstructionListIterator&
  operator=(const InstructionListIterator& it) = default;

  bool operator==(const InstructionListIterator rhs) const {
    return this->curr_ == rhs.curr_;
  }
  bool operator!=(const InstructionListIterator rhs) const {
    return this->curr_ != rhs.curr_;
  }

  Instruction* operator*() const { return this->curr_; }
  Instruction* operator->() const { return this->curr_; }

  InstructionListIterator& operator++() {
    this->curr_ = this->curr_->next;
    return *this;
  }
  InstructionListIterator operator++(int) {
    InstructionListIterator tmp = *this;
    ++this->curr_;
    return tmp;
  }

  InstructionListIterator& operator--() {
    --this->curr_;
    return *this;
  }
  InstructionListIterator operator--(int) {
    InstructionListIterator tmp = *this;
    --this->curr_;
    return tmp;
  }

  bool operator<(const InstructionListIterator rhs) const {
    return this->curr_ < rhs.curr_;
  }
  bool operator>(const InstructionListIterator rhs) const {
    return this->curr_ > rhs.curr_;
  }
  bool operator<=(const InstructionListIterator rhs) const {
    return this->curr_ <= rhs.curr_;
  }
  bool operator>=(const InstructionListIterator rhs) const {
    return this->curr_ >= rhs.curr_;
  }
};
struct InstructionList {
  Instruction* head;
  Instruction* tail;
  bool owning;

  InstructionList() : head(nullptr), tail(nullptr), owning(true) {}
  // ~InstructionList() {
  //   if (!owning)
  //     return;

  //   Instruction *curr = head;
  //   while (curr != nullptr) {
  //     Instruction *tmp = curr;
  //     curr = curr->next;
  //     delete tmp;
  //   }
  // }
  InstructionList(const InstructionList& other) {
    this->head = other.head;
    this->tail = other.tail;
    this->owning = false;
  }
  InstructionList(InstructionList&& other) noexcept = default;
  InstructionList& operator=(const InstructionList& other) {
    this->head = other.head;
    this->tail = other.tail;
    this->owning = false;
    return *this;
  }
  InstructionList& operator=(InstructionList&& other) noexcept = default;

  bool isEmpty() { return this->head == nullptr && this->tail == nullptr; }
  void addBack(Instruction* inst) {
    assert(inst != nullptr);

    if(isEmpty()) {
      head = inst;
      tail = head;
    } else {

      tail->next = inst;
      inst->prev = tail;
      tail = inst;
    }
  }
  void addBack(InstructionList* il) {
    assert(il != nullptr);

    if(isEmpty()) {
      head = il->head;
      tail = il->tail;
    } else {
      tail->next = il->head;
      il->head->prev = tail;
      tail = il->tail;
    }

    il->owning = false;
  }
  void addFront(Instruction* inst) {
    assert(inst != nullptr);
    if(isEmpty()) {
      head = inst;
      tail = head;
    } else {
      inst->next = head;
      head->prev = inst;
      head = inst;
    }
  }
  void addFront(InstructionList* il) {
    assert(il != nullptr);

    if(isEmpty()) {
      head = il->head;
      tail = il->tail;
    } else {
      il->tail->next = head;
      head->prev = il->tail;
      head = il->head;
    }

    il->owning = false;
  }
  void addAfter(Instruction* inst, Instruction* after) {
    assert(inst != nullptr && after != nullptr && !isEmpty());
    if(after == tail) addBack(inst);
    else {
      auto tmp = after->next;
      after->next = inst;
      inst->prev = after;
      inst->next = tmp;
      tmp->prev = inst;
    }
  }
  void addAfter(InstructionList* il, Instruction* after) {
    assert(il != nullptr && after != nullptr && !isEmpty());

    if(after == tail) addBack(il);
    else {

      auto tmp = after->next;
      after->next = il->head;
      il->head->prev = after;
      il->tail->next = tmp;
      tmp->prev = il->tail;
    }

    il->owning = false;
  }

  void addBefore(Instruction* inst, Instruction* before) {
    assert(inst != nullptr && before != nullptr && !isEmpty());

    if(before == head) addFront(inst);
    else {

      auto tmp = before->prev;
      before->prev = inst;
      inst->next = before;
      inst->prev = tmp;
      tmp->next = inst;
    }
  }

  //  after concat, l1 and l2 no longer own the elements
  static InstructionList
  concat(InstructionList& list1, InstructionList& list2) {
    InstructionList il;
    il.head = list1.head;
    il.tail = list2.tail;
    il.owning = true;
    list1.owning = false;
    list2.owning = false;

    // both lists had elements, link them together
    if(!list1.isEmpty() && !list2.isEmpty()) {

      // now need to link list1.tail and list2.head
      list1.tail->next = list2.head;
      list2.head->prev = list1.tail;
    }
    // just list1 empty, tweak head/tail
    else if(list1.isEmpty() && !list2.isEmpty()) {
      il.head = list2.head;
    }
    // just list2 empty, tweak head/tail
    else if(!list1.isEmpty() && list2.isEmpty()) {
      il.tail = list1.tail;
    }

    return il;
  }

  InstructionListIterator begin() { return InstructionListIterator(this); }
  InstructionListIterator end() {
    return InstructionListIterator(this, nullptr); // this feels fragile
  }
};

template <size_t Parameters, size_t Locals> struct Function {
  Types retType;
  std::array<Types, Parameters> argTypes;
  std::array<Variable, Parameters + Locals> variables;
  InstructionList instructions;
  std::string signature(std::string name);
};

class State;

class CompiledRegex {
public:
  CompiledRegex() {
    // function signature
    func.retType = Types::LONG;
    func.argTypes[0] = Types::CONST_CHAR_STAR;
    func.argTypes[1] = Types::LONG;

    //  parameters
    *_input() = Variable::buildParameter(
        func.argTypes[0],
        "input",
        argumentRegisters[0]);
    *_length() = Variable::buildParameter(
        func.argTypes[1],
        "length",
        argumentRegisters[1]);

    // locals
    *_counter() =
        Variable::buildLocal(Types::LONG, "counter", 0, argumentRegisters[2]);
    *_longestMatch() =
        Variable::buildLocal(Types::LONG, "longestMatch", -1, returnRegister);
    *_next() = Variable::buildLocal(Types::CHAR, "next", argumentRegisters[3]);

    doneState = new Return();
    ((Return*)doneState)->value = _longestMatch();
    func.instructions.addBack(doneState);

    insertPoint = nullptr;
  }

public:
  std::string toC(std::string name = "match");
  std::string toNasm(std::string name = "match");
  std::string toHeader(std::string name = "match");

public:
  InstructionList* getNewBlockForState(State*);
  void finishBlock(InstructionList* il);

  Instruction* storeMatch();
  Instruction* matchChar(char c, Instruction* jumpTo);

private:
  Function<2, 3> func;
  Instruction* doneState;
  Instruction* insertPoint;
  Variable* _input() { return &func.variables[0]; }
  Variable* _length() { return &func.variables[1]; }
  Variable* _counter() { return &func.variables[2]; }
  Variable* _longestMatch() { return &func.variables[3]; }
  Variable* _next() { return &func.variables[4]; }
};

#endif
