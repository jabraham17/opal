

#include "parse_regex.h"

// using OwningStateList = std::vector<std::unique_ptr<State>>;
// using StatesStartAccept = std::tuple<OwningStateList, State*, State*>;
// returns start and accept
std::optional<StateList> Parser::parse_expr(
    std::string input,
    size_t& offset,
    std::function<void(std::string_view msg)> errFunc) {
  char next = input[offset++];

  if(next == '(') {
    auto lhs = parse_expr(input, offset, errFunc);
    if(!lhs) return {};

    decltype(lhs) rhs;

    if(input[offset++] != ')') {
      if(errFunc) errFunc("unmatched paren");
      return {};
    }
    char op = input[offset++];

    if(op == '.') {
      if(input[offset++] != '(') {
        if(errFunc) errFunc("expected paren");
        return {};
      }
      rhs = parse_expr(input, offset, errFunc);
      if(!rhs) return {};
      if(input[offset++] != ')') {
        if(errFunc) errFunc("unmatched paren");
        return {};
      }

      // do concat
      // lhs.accept -> rhs.start
      // unmark lhs.accept as an accept
      // return {lhs.start, rhs.accept}

      // OwningStateList& lhs_states = std::get<0>(*lhs);
      // OwningStateList& rhs_states = std::get<0>(*rhs);

      // State* lhs_start = std::get<1>(*lhs);
      // State* lhs_accept = std::get<2>(*lhs);
      // State* rhs_start = std::get<1>(*rhs);
      // State* rhs_accept = std::get<2>(*rhs);
      // lhs_accept->setAccept(false);
      // lhs_accept->addTransition(rhs_start);

      // OwningStateList states;
      // states.insert()

      lhs->concatWith(*rhs);
      return lhs;

    } else if(op == '|') {
      if(input[offset++] != '(') {
        if(errFunc) errFunc("expected paren");
        return {};
      }
      rhs = parse_expr(input, offset, errFunc);
      if(!rhs) return {};
      if(input[offset++] != ')') {
        if(errFunc) errFunc("unmatched paren");
        return {};
      }

      lhs->unionWith(*rhs);
      return lhs;

    } else if(op == '*') {
      lhs->kleene();
      return lhs;
    } else {
      if(errFunc) errFunc("unknown op");
      return {};
    }

  } else {
    // consume the char
    auto start = std::make_unique<State>();
    auto accept = std::make_unique<State>(true);
    start->addTransition(accept.get(), next);

    auto start_ptr = start.get();
    auto accept_ptr = accept.get();

    StateList sl;
    sl.addEntry(std::move(start));
    sl.add(std::move(accept));

    // OwningStateList states;
    // states.push_back(std::move(start));
    // states.push_back(std::move(accept));
    return sl;
  }
}
