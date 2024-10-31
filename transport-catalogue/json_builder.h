#pragma once

#include <string>
#include <vector>
#include <optional>

#include "json.h"

namespace json{
    
class Builder;
class KeyItemContext;
class DictContext;
class ArrayContext;

class Builder {
public:
    KeyItemContext Key(const std::string& key);
    Builder& Value(Node::Value value);
    DictContext StartDict();
    ArrayContext StartArray();
    Builder& EndDict();
    Builder& EndArray();
    Node Build();

private:
    std::optional<Node> root_;
    std::vector<Node*> nodes_stack_;
    std::optional<std::string> current_key_;
    
    void AddNode(Node node);
};
    
class Context {
public:
    explicit Context(Builder& builder) : builder_(builder) {}

protected:
    Builder& builder_;
};
    
class KeyItemContext : public Context {
public:
    explicit KeyItemContext(Builder& builder) : Context(builder) {}
    DictContext Value(Node::Value value);
    DictContext StartDict();
    ArrayContext StartArray();
};

class DictContext : public Context {
public:
    explicit DictContext(Builder& builder) : Context(builder) {}
    KeyItemContext Key(const std::string& key);
    Builder& EndDict();
};

class ArrayContext : public Context {
public:
    explicit ArrayContext(Builder& builder) : Context(builder) {}
    ArrayContext Value(Node::Value value);
    DictContext StartDict();
    ArrayContext StartArray();
    Builder& EndArray();
};
    
}