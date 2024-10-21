#pragma once

#include <cassert>
#include <iostream>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <variant>
#include <vector>

namespace json {

class Node;
using Dict = std::map<std::string, Node>;
using Array = std::vector<Node>;

class ParsingError : public std::runtime_error {
public:
    using runtime_error::runtime_error;
};

class Node {
public:
    using Value = std::variant<std::nullptr_t, Array, Dict, bool, int, double, std::string>;
        
    Node();
    Node(std::nullptr_t);
    Node(const Array& array);
    Node(const Dict& dict);
    Node(bool b);
    Node(int i);
    Node(double d);
    Node(const std::string& str);
    
    const Value& GetValue() const;
    
    bool IsNull() const;
    bool IsArray() const;
    bool IsMap() const;
    bool IsBool() const;
    bool IsInt() const;
    bool IsPureDouble() const;
    bool IsDouble() const;
    bool IsString() const;
    
    const Array& AsArray() const;
    const Dict& AsMap() const;
    bool AsBool() const;
    int AsInt() const;
    double AsDouble() const;
    const std::string& AsString() const;
    
    bool operator==(const Node& other) const;
    bool operator!=(const Node& other) const;

private:
    Value value_;
};

class Document {
public:
    explicit Document();
    explicit Document(Node root);

    const Node& GetRoot() const;
    
    bool operator==(const Document& other) const;
    bool operator!=(const Document& other) const;

private:
    Node root_;
};

Document Load(std::istream& input);

void Print(const Document& doc, std::ostream& output);

}  // namespace json