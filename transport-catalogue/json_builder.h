#pragma once

#include "json.h"

#include <optional>


namespace json {

    class Builder {
    public:
        class DictContext;
        class KeyContext;
        class ArrayContext;

        Builder();
        KeyContext Key(std::string key);
        Builder& Value(Node::Value value);
        DictContext StartDict();
        ArrayContext StartArray();
        Builder& EndDict();
        Builder& EndArray();
        Node Build();
        Node AddNode(Node::Value value);

    private:
        Node root_{ nullptr };
        std::vector<Node*> nodes_stack_;
        std::optional<std::string> key_;

        void ValidateKey();
        void AddValueToDict(Node* top_node, Node::Value value);
        void AddValueToArray(Node* top_node, Node::Value value);
        void CreateNewContainerInDict(Node* top_node, bool is_array);
        void CreateNewContainerInArray(Node* top_node, bool is_array);
        void ValidateTopNodeIsDict();
        void ValidateTopNodeIsArray();

    };

    class Builder::DictContext {
    public:
        DictContext(Builder& builder);
        KeyContext Key(std::string key);
        Builder& EndDict();

        DictContext StartDict() = delete;
        ArrayContext StartArray() = delete;
        Builder& EndArray() = delete;
        Builder& Value(Node::Value value) = delete;
    private:
        Builder& builder_;
    };

    class Builder::ArrayContext {
    public:
        ArrayContext(Builder& builder);
        ArrayContext Value(Node::Value value);
        DictContext StartDict();
        Builder& EndArray();
        ArrayContext StartArray();

        KeyContext Key(std::string key) = delete;
        Builder& EndDict() = delete;
    private:
        Builder& builder_;
    };

    class Builder::KeyContext {
    public:
        KeyContext(Builder& builder);
        DictContext Value(Node::Value value);
        ArrayContext StartArray();
        DictContext StartDict();

        Builder& EndDict() = delete;
        Builder& EndArray() = delete;
        KeyContext Key(std::string key) = delete;
    private:
        Builder& builder_;
    };

} // namespace json
