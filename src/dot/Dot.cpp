#include "Dot.h"

#include "common/utils.h"

#include <algorithm>
#include <sstream>
namespace dot {

static std::string attrsToString(
    Graph::AttributeMap attrs,
    std::string_view prefix = "[",
    std::string_view postfix = "]") {
  std::stringstream ss;
  for(auto [k, v] : attrs) {
    ss << prefix << std::string(k) << "=" << std::string(v) << postfix;
  }
  return ss.str();
}

static std::string removeSpaces(std::string_view s) {
  std::string noSpace(s);
  std::transform(noSpace.begin(), noSpace.end(), noSpace.begin(), [](char c) {
    if(c == ' ') return '_';
    else return c;
  });
  return noSpace;
}

std::string Graph::toString(std::size_t indent) {
  std::stringstream ss;

  if(this->directed() && !this->isSubgraph()) ss << "di";
  if(this->isSubgraph()) ss << "sub";

  std::string noSpaceName = removeSpaces(this->name());

  std::string niceName;
  std::string refName = noSpaceName;
  if(this->isSubgraph()) {
    niceName = "cluster_" + noSpaceName;
    refName = niceName;
  } else {
    niceName = "\"" + this->name() + "\"";
  }
  ss << "graph " << niceName + "{\n";

  indent += 2;

  ss << attrsToString(this->attributes(), std::string(indent, ' '), ";\n");

  for(const auto& v : this->vertexes()) {
    ss << std::string(indent, ' ');
    ss << refName << "_" << intptr_t(v.get());
    ss << attrsToString(v->attributes());
    ss << ";\n";
  }

  std::string arrow = this->directed() ? "->" : "--";
  for(const auto& e : this->edges()) {
    ss << std::string(indent, ' ');
    ss << refName << "_" << intptr_t(e->first) << arrow << refName << "_"
       << intptr_t(e->second);
    ss << attrsToString(e->attributes());
    ss << ";\n";
  }

  for(const auto& c : this->children()) {
    ss << c->toString(indent);
  }

  indent -= 2;
  ss << std::string(indent, ' ') + "}\n";
  return ss.str();
}

} // namespace dot
