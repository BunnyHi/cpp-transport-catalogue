#include "json_builder.h"

namespace json {

    Builder::Builder() : root_{ nullptr }, key_{ std::nullopt } {
        nodes_stack_.emplace_back(&root_);
    }

    Builder::KeyContext Builder::Key(std::string key) {
        ValidateKey();
        key_ = std::move(key);
        return *this;
    }

    Builder& Builder::Value(Node::Value value) {
        auto* top_node = nodes_stack_.back();
        if (top_node->IsMap()) {
            AddValueToDict(top_node, std::move(value));
        }
        else if (top_node->IsArray()) {
            AddValueToArray(top_node, std::move(value));
        }
        else if (root_.IsNull()) {
            root_.GetValue() = std::move(value);
        }
        else {
            throw std::logic_error("Cannot add value: The current context is not a valid container");
        }
        return *this;
    }

    Builder::DictContext Builder::StartDict() {
        auto* top_node = nodes_stack_.back();
        if (top_node->IsMap()) {
            CreateNewContainerInDict(top_node, false); 
        }
        else if (top_node->IsArray()) {
            CreateNewContainerInArray(top_node, false);
        }
        else if (top_node->IsNull()) {
            top_node->GetValue() = Dict();
        }
        else {
            throw std::logic_error("Invalid state: Cannot start a dictionary from the current node");
        }
        return *this;
    }

    Builder::ArrayContext Builder::StartArray() {
        auto* top_node = nodes_stack_.back();
        if (top_node->IsMap()) {
            CreateNewContainerInDict(top_node, true); 
        }
        else if (top_node->IsArray()) {
            CreateNewContainerInArray(top_node, true);
        }
        else if (top_node->IsNull()) {
            top_node->GetValue() = Array();
        }
        else {
            throw std::logic_error("Invalid state: Cannot start an array from the current node");
        }
        return *this;
    }

    Builder& Builder::EndDict() {
        ValidateTopNodeIsDict();
        nodes_stack_.pop_back();
        key_.reset();
        return *this;
    }

    Builder& Builder::EndArray() {
        ValidateTopNodeIsArray();
        nodes_stack_.pop_back();
        return *this;
    }

    Node Builder::Build() {
        if (root_.IsNull() || nodes_stack_.size() > 1) {
            throw std::logic_error("Invalid build: The root is empty or not all containers are closed properly");
        }
        return root_;
    }

    Node Builder::AddNode(Node::Value value) {
        return std::visit([](auto&& arg) -> Node {
            return Node(std::forward<decltype(arg)>(arg));
            }, value);
    }

    void Builder::ValidateKey() {
        if (!nodes_stack_.back()->IsMap()) {
            throw std::logic_error("Invalid context: Key is not allowed in the current node type.");
        }
        if (key_) {
            throw std::logic_error("Invalid operation: Key is already set and cannot be overwritten.");
        }
    }

    void Builder::AddValueToDict(Node* top_node, Node::Value value) {
        if (!key_) {
            throw std::logic_error("Cannot add value: A key must be set before adding a value to a dictionary.");
        }
        auto& dict = std::get<Dict>(top_node->GetValue());
        auto [position, success] = dict.emplace(std::move(*key_), Node{});
        key_.reset();
        position->second.GetValue() = std::move(value);
    }

    void Builder::AddValueToArray(Node* top_node, Node::Value value) {
        auto& array = std::get<Array>(top_node->GetValue());
        array.emplace_back(AddNode(value));
    }

    void Builder::CreateNewContainerInDict(Node* top_node, bool is_array) {
        if (!key_) {
            throw std::logic_error("Cannot create a new container: A key must be set first.");
        }

        auto& dict = std::get<Dict>(top_node->GetValue());
        Node new_container = is_array ? AddNode(Array()) : AddNode(Dict());
        auto [position, success] = dict.emplace(std::move(*key_), std::move(new_container));
        key_.reset();
        nodes_stack_.emplace_back(&position->second);
    }

    void Builder::CreateNewContainerInArray(Node* top_node, bool is_array) {
        auto& array = std::get<Array>(top_node->GetValue());

        if (is_array) {
            array.emplace_back(Array());
        }
        else {
            array.emplace_back(Dict());
        }

        nodes_stack_.emplace_back(&array.back());
    }

    void Builder::ValidateTopNodeIsDict() {
        if (!nodes_stack_.back()->IsMap()) {
            throw std::logic_error("Expected a dictionary at the top of the stack, but found a different type.");
        }
    }

    void Builder::ValidateTopNodeIsArray() {
        if (!nodes_stack_.back()->IsArray()) {
            throw std::logic_error("Expected an array at the top of the stack, but found a different type.");
        }
    }

    //================================= Вспомогательные классы состояния =================================

    Builder::DictContext::DictContext(Builder& builder)
        : builder_(builder)
    {
    }

    Builder::KeyContext Builder::DictContext::Key(std::string key) {
        return builder_.Key(std::move(key));
    }

    Builder& Builder::DictContext::EndDict() {
        return builder_.EndDict();
    }

    Builder::ArrayContext::ArrayContext(Builder& builder)
        : builder_(builder)
    {
    }

    Builder::ArrayContext Builder::ArrayContext::Value(Node::Value value) {
        return ArrayContext(builder_.Value(std::move(value)));
    }

    Builder::DictContext Builder::ArrayContext::StartDict() {
        return builder_.StartDict();
    }

    Builder::ArrayContext Builder::ArrayContext::StartArray() {
        return builder_.StartArray();
    }

    Builder& Builder::ArrayContext::EndArray() {
        return builder_.EndArray();
    }

    Builder::KeyContext::KeyContext(Builder& builder)
        : builder_(builder)
    {
    }

    Builder::DictContext Builder::KeyContext::Value(Node::Value value) {
        return DictContext(builder_.Value(std::move(value)));
    }

    Builder::ArrayContext Builder::KeyContext::StartArray() {
        return builder_.StartArray();
    }

    Builder::DictContext Builder::KeyContext::StartDict() {
        return builder_.StartDict();
    }

} // namespace json
