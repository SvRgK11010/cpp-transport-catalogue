#pragma once
#include "json.h"
#include <optional>
#include <memory>

namespace json {

	class Builder {
	private:
		class BaseContext;
		class DictItemContext;
		class ArrayItemContext;
		class KeyItemContext;
	public:
		Builder();

		KeyItemContext Key(std::string key);
		BaseContext Value(Node::Value value);
		DictItemContext StartDict();
		ArrayItemContext StartArray();
		Builder& EndDict();
		Builder& EndArray();

		Node Build();
		Node GetNode(Node::Value value);

	private:
		Node root_{ nullptr };
		std::vector<Node*> nodes_stack_;
		std::optional<std::string> key_{std::nullopt};

		class BaseContext {
		public:
			BaseContext(Builder& builder)
				: builder_(builder) {}

			Node Build();
			KeyItemContext Key(std::string key);
			BaseContext Value(Node::Value value);
			DictItemContext StartDict();
			Builder& EndDict();
			ArrayItemContext StartArray();
			Builder& EndArray();
		private:
			Builder& builder_;
		};
		class DictItemContext : public BaseContext {
		public:
			DictItemContext(BaseContext base) : BaseContext(base) {}

			KeyItemContext Key(std::string key);
			Builder& EndDict();

			Node Build() = delete;
			DictItemContext Value(Node::Value value) = delete;
			DictItemContext StartDict() = delete;
			ArrayItemContext StartArray() = delete;
			Builder& EndArray() = delete;

		};

		class ArrayItemContext : public BaseContext {
		public:
			ArrayItemContext(BaseContext base) : BaseContext(base) {}

			ArrayItemContext Value(Node::Value value);
			DictItemContext StartDict();
			ArrayItemContext StartArray();
			Builder& EndArray();

			Node Build() = delete;
			KeyItemContext Key(std::string key) = delete;
			Builder& EndDict() = delete;
		};

		class KeyItemContext : public BaseContext {
		public:
			KeyItemContext(BaseContext base) : BaseContext(base) {}

			DictItemContext Value(Node::Value value);
			DictItemContext StartDict();
			ArrayItemContext StartArray();

			Node Build() = delete;
			KeyItemContext Key(std::string key) = delete;
			Builder& EndDict() = delete;
			Builder& EndArray() = delete;
		};
	};
}