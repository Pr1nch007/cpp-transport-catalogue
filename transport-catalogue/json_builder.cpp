#include "json_builder.h"

namespace json{

KeyItemContext Builder::Key(const std::string& key) {
    if (nodes_stack_.empty() || !nodes_stack_.back()->IsMap()) {
        throw std::logic_error("Key method called outside of a dictionary context");
    }
    current_key_ = key;
    return KeyItemContext(*this);
}
    
Builder& Builder::Value(Node::Value value) {
    if (nodes_stack_.empty()) {
        root_ = Node(std::move(value));
    } else if (nodes_stack_.back()->IsArray()) {
        std::get<Array>(*nodes_stack_.back()).emplace_back(std::move(value));
    } else if (nodes_stack_.back()->IsMap()) {
        std::get<Dict>(*nodes_stack_.back()).emplace(*current_key_, Node(std::move(value)));
        current_key_.reset();
    }
    return *this;
}
    
DictContext Builder::StartDict() {
    AddNode(Node(Dict{}));
    return DictContext(*this);
}
    
Builder& Builder::EndDict() {
    if (nodes_stack_.empty() || !nodes_stack_.back()->IsMap()) {
        throw std::logic_error("EndDict called without a matching StartDict");
    }
    nodes_stack_.pop_back();
    return *this;
}
    
ArrayContext Builder::StartArray() {
    AddNode(Node(Array{}));
    return ArrayContext(*this);
}
    
Builder& Builder::EndArray() {
    if (nodes_stack_.empty() || !nodes_stack_.back()->IsArray()) {
        throw std::logic_error("EndArray called without a matching StartArray");
    }
    nodes_stack_.pop_back();
    return *this;
}
    
Node Builder::Build() {
    if (!nodes_stack_.empty()) {
        throw std::logic_error("Unmatched StartArray or StartDict calls");
    }
    return *root_;
}
    
void Builder::AddNode(Node node) {
    if (!nodes_stack_.empty()) {
        Node& top = *nodes_stack_.back();
        if (top.IsArray()) {
            std::get<Array>(top).emplace_back(std::move(node));
            nodes_stack_.push_back(&std::get<Array>(top).back());
        } else if (top.IsMap()) {
            if (!current_key_) {
                throw std::logic_error("Adding node to dictionary requires a key");
            }
            std::get<Dict>(top)[*current_key_] = std::move(node);
            nodes_stack_.push_back(&std::get<Dict>(top)[*current_key_]);
            current_key_.reset();
        }
    } else if (!root_) {
        root_ = std::move(node);
        nodes_stack_.push_back(&*root_);
    } else {
        throw std::logic_error("Builder is already complete");
    }
}
    
DictContext KeyItemContext::Value(Node::Value value) {
    builder_.Value(std::move(value));
    return DictContext(builder_);
}
    
DictContext KeyItemContext::StartDict() {
    return builder_.StartDict();
}
    
ArrayContext KeyItemContext::StartArray() {
    return builder_.StartArray();
}

KeyItemContext DictContext::Key(const std::string& key) {
    return builder_.Key(key);
}

Builder& DictContext::EndDict() {
    return builder_.EndDict();
}

ArrayContext ArrayContext::Value(Node::Value value) {
    builder_.Value(std::move(value));
    return *this;
}

DictContext ArrayContext::StartDict() {
    return builder_.StartDict();
}

ArrayContext ArrayContext::StartArray() {
    return builder_.StartArray();
}

Builder& ArrayContext::EndArray() {
    return builder_.EndArray();
}
    
}