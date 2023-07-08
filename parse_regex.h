

#include "DFA.h"

#include <optional>
// #include <tuple>
#include <functional>
#include <string>
/*simple, easy to parse grammar
later we can build a simple push down stack parser without . using precedence
_ is epsilon

expr -> CHAR
expr -> _
expr -> (expr).(expr)
expr -> (expr)|(expr)
expr -> (expr)*

*/

class Parser {
public:
  std::optional<StateList> parse(
      std::string input,
      std::function<void(std::string_view msg)> errFunc = {}) {
    size_t offset = 0;
    auto res = parse_expr(input, offset, errFunc);
    if(offset == input.size() && res) return res;
    if(errFunc && offset != input.size()) {
      errFunc(
          "did not match full input " + std::to_string(offset) +
          "!=" + std::to_string(input.size()));
    }
    return {};
  }

private:
  // using OwningStateList = std::vector<std::unique_ptr<State>>;
  // using StatesStartAccept = std::tuple<OwningStateList, State*, State*>;
  // returns start and accept
  std::optional<StateList> parse_expr(
      std::string input,
      size_t& offset,
      std::function<void(std::string_view msg)> errFunc);
};
