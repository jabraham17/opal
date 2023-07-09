#include "Instruction.h"

#include "DFA.h"

#include <sstream>

InstructionListIterator::InstructionListIterator(InstructionList* list)
    : list(list), curr_(list->head) {}

std::string toString(Types t) {
  if(t == Types::CHAR) return "char";
  if(t == Types::CHAR_STAR) return "char*";
  if(t == Types::CONST_CHAR_STAR) return "const char*";
  if(t == Types::LONG) return "long";
  if(t == Types::INT) return "int";
  return "<unknown>";
}
std::string toString(ConditionCode cc, bool isAsm) {
  if(cc == ConditionCode::EQ) return !isAsm ? "==" : "e";
  if(cc == ConditionCode::NEQ) return !isAsm ? "!=" : "ne";
  if(cc == ConditionCode::GT) return !isAsm ? ">" : "g";
  if(cc == ConditionCode::GTEQ) return !isAsm ? ">=" : "ge";
  if(cc == ConditionCode::LT) return !isAsm ? "<" : "l";
  if(cc == ConditionCode::LTEQ) return !isAsm ? "<=" : "le";
  return "<unknown>";
}

std::string toString(X86Register reg, Types t) {
  switch(reg) {
    case X86Register::DI:
      switch(t) {
        case Types::CHAR: return "dil";
        case Types::CHAR_STAR: return "rdi";
        case Types::CONST_CHAR_STAR: return "rdi";
        case Types::LONG: return "rdi";
        case Types::INT: return "edi";
      }
    case X86Register::SI:
      switch(t) {
        case Types::CHAR: return "sil";
        case Types::CHAR_STAR: return "rsi";
        case Types::CONST_CHAR_STAR: return "rsi";
        case Types::LONG: return "rsi";
        case Types::INT: return "esi";
      }
    case X86Register::D:
      switch(t) {
        case Types::CHAR: return "dl";
        case Types::CHAR_STAR: return "rdx";
        case Types::CONST_CHAR_STAR: return "rdx";
        case Types::LONG: return "rdx";
        case Types::INT: return "edx";
      }
    case X86Register::A:
      switch(t) {
        case Types::CHAR: return "al";
        case Types::CHAR_STAR: return "rax";
        case Types::CONST_CHAR_STAR: return "rax";
        case Types::LONG: return "rax";
        case Types::INT: return "eax";
      }
    case X86Register::C:
      switch(t) {
        case Types::CHAR: return "cl";
        case Types::CHAR_STAR: return "rcx";
        case Types::CONST_CHAR_STAR: return "rcx";
        case Types::LONG: return "rcx";
        case Types::INT: return "ecx";
      }
    default: return "<unknown>";
  }
}

std::array argumentRegisters = {
    X86Register::DI,
    X86Register::SI,
    X86Register::D,
    X86Register::C,
    X86Register::R8,
    X86Register::R9};
X86Register returnRegister = X86Register::A;
std::array tmpRegisters = {X86Register::R10, X86Register::R11};

std::string loadExtForType(Types destType, Types loadType) {
  if(destType == loadType) return ""; // no extension needed
  if(loadType == Types::CHAR) return "zx";
  return "<unknown>";
}
std::string loadSizeForType(Types loadType) {
  if(loadType == Types::CHAR) return "byte";
  return "<unknown>";
}

std::string CompiledRegex::toNasm(std::string name) {

  std::stringstream ss;

  ss << "bits 64\n";
  ss << "section .text\n";

  // func header
  ss << "global " << name << "\n";
  ss << name << ":\n";

  // variable init
  for(auto& p : func.variables) {
    if(p->isLocal() && p->hasInitialValue()) {
      auto val = p->initialValue;
      // if zero, do an xor of the 32 bit version
      if(val == 0) {
        auto r = toString(p->reg, Types::INT);
        ss << "  xor " << r << ", " << r << "\n";
      } else {
        auto r = toString(p->reg, p->type);
        ss << "  mov " << r << ", " << val << "\n";
      }
    }
  }

  // // code gen functions
  for(auto i : func.instructions) {
    ss << i->toNasm() << "\n";
  }

  return ss.str();
}

template <size_t Parameters, size_t Locals>
std::string Function<Parameters, Locals>::signature(std::string name) {
  std::stringstream ss;
  // func header
  ss << toString(retType) << " " << name << "(";
  std::string sep;
  for(auto& p : variables) {
    if(p->isParameter()) {
      ss << sep << toString(p->type) << " " << p->name;
      sep = ", ";
    }
  }
  ss << ")";
  return ss.str();
}

std::string CompiledRegex::toHeader(std::string name) {
  std::string ret = func.signature(name) + ";";
  return ret;
}
std::string CompiledRegex::toC(std::string name) {

  std::stringstream ss;

  ss << func.signature(name) << " {\n";

  // variable init
  for(auto& p : func.variables) {
    if(p->isLocal()) {
      ss << "  " << toString(p->type) << " " << p->name;
      if(p->hasInitialValue()) ss << " = " << p->getInitialValue();
      ss << ";\n";
    }
  }

  // code gen functions
  for(auto i : func.instructions) {
    ss << i->toC();
  }
  ss << "}";
  return ss.str();
}

// we can prob make this unique ptr?
InstructionList* CompiledRegex::getNewBlockForState(State* state) {
  InstructionList* il = new InstructionList();

  // first state is a nop
  il->addBack(new NOP());

  // check for i >= n
  ConditionalJump* cjmp = new ConditionalJump;
  cjmp->lhs = counter_().get();
  cjmp->rhs = length_().get();
  cjmp->cc = ConditionCode::GTEQ;
  cjmp->target = doneState;
  il->addBack(cjmp);

  // read char
  Load* load = new Load;
  load->base = input_().get();
  load->offset = counter_().get();
  load->loadType = Types::CHAR;
  load->dest = next_().get();
  il->addBack(load);

  // inc counter
  Add* add = new Add;
  add->dest = counter_().get();
  add->op1 = Variable::buildImmediate(Types::LONG, 1);
  il->addBack(add);

  return il;
}
// this can consume the unique_ptr and dispose of memory?
void CompiledRegex::finishBlock(InstructionList* il) {
  // add final jump to done and concat into the function

  Jump* jmp = new Jump;
  jmp->target = doneState;
  il->addBack(jmp);

  if(insertPoint) {
    func.instructions.addAfter(il, insertPoint);
  } else {
    func.instructions.addFront(il);
  }

  // reset insert point to end of finished block
  insertPoint = jmp;
}

Instruction* CompiledRegex::storeMatch() {
  Copy* inst = new Copy;
  inst->source = counter_().get();
  inst->dest = longestMatch_().get();
  return inst;
}
Instruction* CompiledRegex::matchChar(char c, Instruction* jumpTo) {
  ConditionalJump* cjmp = new ConditionalJump;
  cjmp->lhs = next_().get();
  cjmp->rhs = Variable::buildImmediate(Types::CHAR, c);
  cjmp->cc = ConditionCode::EQ;
  cjmp->target = jumpTo;

  return cjmp;
}
