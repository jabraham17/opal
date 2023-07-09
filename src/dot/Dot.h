#ifndef OPAL_DOT_DOT_H_
#define OPAL_DOT_DOT_H_

#ifndef _SIMPLEASSEMBLER_DOTWRITER_H_
#define _SIMPLEASSEMBLER_DOTWRITER_H_

#include <map>
#include <memory>
#include <optional>
#include <string>
#include <vector>
namespace dot {

class Graph {

public:
  using AttributeKey = std::string;
  using AttributeValue = std::string;
  using AttributeMap = std::map<AttributeKey, AttributeValue>;
  struct Vertex {
  private:
    friend Graph;
    AttributeMap attrs;

  public:
    Vertex() : attrs() {}
    Vertex(AttributeMap attrs) : attrs(attrs) {}

    AttributeMap attributes() { return this->attrs; }
    void setAttribute(const AttributeKey& k, AttributeValue v) {
      this->attrs.insert_or_assign(k, v);
    }
    bool hasAttribute(const AttributeKey& k) {
      return this->attrs.count(k) == 1;
    }
    std::optional<AttributeValue> getAttribute(const AttributeKey& k) {
      auto it = this->attrs.find(k);
      if(it != this->attrs.end()) return it->second;
      return std::nullopt;
    }
    void removeAttribute(const AttributeKey& k) {
      auto it = this->attrs.find(k);
      if(it != this->attrs.end()) this->attrs.erase(it);
    }
  };

  struct Edge {
  private:
    friend Graph;
    Vertex* first;
    Vertex* second;
    AttributeMap attrs;

  public:
    Edge(Vertex* first, Vertex* second)
        : first(first), second(second), attrs() {}
    Edge(Vertex* first, Vertex* second, AttributeMap attrs)
        : first(first), second(second), attrs(attrs) {}

    AttributeMap attributes() { return this->attrs; }
    void setAttribute(const AttributeKey& k, AttributeValue v) {
      this->attrs.insert_or_assign(k, v);
    }
    bool hasAttribute(const AttributeKey& k) {
      return this->attrs.count(k) == 1;
    }
    std::optional<AttributeValue> getAttribute(const AttributeKey& k) {
      auto it = this->attrs.find(k);
      if(it != this->attrs.end()) return it->second;
      return std::nullopt;
    }
    void removeAttribute(const AttributeKey& k) {
      auto it = this->attrs.find(k);
      if(it != this->attrs.end()) this->attrs.erase(it);
    }
  };

private:
  std::string name_;
  bool directed_;
  std::vector<std::unique_ptr<Vertex>> vertexes_;
  std::vector<std::unique_ptr<Edge>> edges_;
  AttributeMap attrs_;
  std::vector<std::unique_ptr<Graph>> children_;
  Graph* parent_;

public:
  Graph(std::string_view name) : Graph(std::string(name)) {}
  Graph(std::string name)
      : name_(name), directed_(false), vertexes_(), edges_(), attrs_(),
        children_(), parent_(nullptr){};

public:
  std::string name() { return this->name_; }
  void setName(std::string name) { this->name_ = name; }

  bool directed() { return this->directed_; }
  void setDirected(bool directed) { this->directed_ = directed; }

  const std::vector<std::unique_ptr<Graph>>& children() {
    return this->children_;
  }
  void addChild(std::unique_ptr<Graph> g) {
    g->parent_ = this;
    this->children_.push_back(std::move(g));
  }
  bool isSubgraph() { return this->parent_ != nullptr; }

  AttributeMap attributes() { return this->attrs_; }
  void setAttribute(const AttributeKey& k, AttributeValue v) {
    this->attrs_.insert_or_assign(k, v);
  }
  bool hasAttribute(const AttributeKey& k) {
    return this->attrs_.count(k) == 1;
  }
  std::optional<AttributeValue> getAttribute(const AttributeKey& k) {
    auto it = this->attrs_.find(k);
    if(it != this->attrs_.end()) return it->second;
    return std::nullopt;
  }
  void removeAttribute(const AttributeKey& k) {
    auto it = this->attrs_.find(k);
    if(it != this->attrs_.end()) this->attrs_.erase(it);
  }

  Vertex* addVertex() {
    this->vertexes_.push_back(std::make_unique<Vertex>());
    auto& vertex = this->vertexes_.back();
    return vertex.get();
  }

  Edge* addEdge(Vertex* first, Vertex* second) {
    this->edges_.push_back(std::make_unique<Edge>(first, second));
    auto& edge = this->edges_.back();
    return edge.get();
  }

  std::string toString(std::size_t indent = 0);

private:
  const std::vector<std::unique_ptr<Vertex>>& vertexes() {
    return this->vertexes_;
  }
  const std::vector<std::unique_ptr<Edge>>& edges() { return this->edges_; }
};

} // namespace dot
#endif //_SIMPLEASSEMBLER_DOTWRITER_H_
#endif
