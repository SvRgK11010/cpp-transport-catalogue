#include "json_builder.h"

namespace json {

	Builder::Builder()
	{
		Node* root_ptr = &root_;
		nodes_stack_.emplace_back(root_ptr);
	}

	Builder::KeyItemContext Builder::Key(std::string key) {
		if (nodes_stack_.back()->IsDict() && key_) {
			throw std::logic_error("impossible to add key: " + key);
		}
		key_ = std::move(key);
		return  BaseContext(*this);

	}

	Builder::BaseContext Builder::Value(Node::Value value) {
		auto* back = nodes_stack_.back();
		if (back->IsDict()) {
			if (!key_) { throw std::logic_error("impossible to add value in the Dict without key"); }
			std::get<Dict>(nodes_stack_.back()->GetValue()).emplace(std::move(key_.value()), GetNode(value));
			key_ = std::nullopt;
		}
		else if (back->IsArray()) {
			std::get<Array>(nodes_stack_.back()->GetValue()).emplace_back(GetNode(value));
		}
		else if (root_.IsNull()) {
			root_.GetValue() = std::move(value);
		}
		else { throw std::logic_error("impossible to add value"); }
		return *this;
	}

	Builder::DictItemContext Builder::StartDict() {
		auto* back = nodes_stack_.back();
		if (back->IsDict()) {
			if (!key_) { throw std::logic_error("impossible to add value-dict in the Dict without key"); }
			auto& back_dict = std::get<Dict>(back->GetValue());
			auto [key, new_dict] = back_dict.emplace(std::move(key_.value()), Dict());
			key_ = std::nullopt;
			nodes_stack_.emplace_back(&key->second);
		}
		else if (back->IsArray()) {
			auto& back_array = std::get<Array>(back->GetValue());
			back_array.emplace_back(Dict());
			nodes_stack_.emplace_back(&back_array.back());
		}
		else if (back->IsNull()) {
			back->GetValue() = Dict();
		}
		else { throw std::logic_error("impossible to start Dict"); }
		BaseContext bc(*this);
		return DictItemContext{ bc };
	}

	Builder::ArrayItemContext Builder::StartArray() {
		auto* back = nodes_stack_.back();
		if (back->IsDict()) {
			if (!key_) { throw std::logic_error("impossible to add value-dict in the Dict without key"); }
			auto& back_dict = std::get<Dict>(back->GetValue());
			auto [key, new_dict] = back_dict.emplace(std::move(key_.value()), Array());
			key_ = std::nullopt;
			nodes_stack_.emplace_back(&key->second);
		}
		else if (back->IsArray()) {
			auto& back_array = std::get<Array>(back->GetValue());
			back_array.emplace_back(Array());
			nodes_stack_.emplace_back(&back_array.back());
		}
		else if (back->IsNull()) {
			back->GetValue() = Array();
		}
		else { throw std::logic_error("impossible to start Array"); }
		BaseContext bc(*this);
		return ArrayItemContext{ bc };
	}

	Builder& Builder::EndDict() {
		if (!nodes_stack_.back()->IsDict()) {
			throw std::logic_error("impossible to complete the dict");
		}
		nodes_stack_.pop_back();
		return *this;
	}

	Builder& Builder::EndArray() {
		if (!nodes_stack_.back()->IsArray()) {
			throw std::logic_error("impossible to complete the array");
		}
		nodes_stack_.pop_back();
		return *this;
	}

	Node Builder::Build() {
		if (root_.IsNull() || nodes_stack_.size() > 1)
			throw std::logic_error("Could not build JSON");

		return root_;
	}
	Node Builder::GetNode(Node::Value value) {
		if (std::holds_alternative<int>(value)) {
			return Node(std::get<int>(value));
		}
		if (std::holds_alternative<double>(value)) {
			return Node(std::get<double>(value));
		}
		if (std::holds_alternative<std::string>(value)) {
			return Node(std::get<std::string>(value));
		}
		if (std::holds_alternative<std::nullptr_t>(value)) {
			return Node(std::get<std::nullptr_t>(value));
		}
		if (std::holds_alternative<bool>(value)) {
			return Node(std::get<bool>(value));
		}
		if (std::holds_alternative<Dict>(value)) {
			return Node(std::get<Dict>(value));
		}
		if (std::holds_alternative<Array>(value)) {
			return Node(std::get<Array>(value));
		}
		return {};
	}

	//===============BaseContext=================
	Node Builder::BaseContext::Build() {
		return builder_.Build();
	}

	Builder::KeyItemContext Builder::BaseContext::Key(std::string key) {
		return builder_.Key(std::move(key));
	}

	Builder::BaseContext Builder::BaseContext::Value(Node::Value value) {
		return builder_.Value(std::move(value));
	}

	Builder::DictItemContext Builder::BaseContext::StartDict() {
		return builder_.StartDict();
	}

	Builder& Builder::BaseContext::EndDict() {
		return builder_.EndDict();
	}

	Builder::ArrayItemContext Builder::BaseContext::StartArray() {
		return builder_.StartArray();
	}

	Builder& Builder::BaseContext::EndArray() {
		return builder_.EndArray();
	}

	//===============DictItemContext=================

	Builder::KeyItemContext Builder::DictItemContext::Key(std::string key) {
		return BaseContext::Key(std::move(key));
	}

	Builder& Builder::DictItemContext::EndDict() {
		return BaseContext::EndDict();
	}

	//===============ArrayItemContext=================

	Builder::ArrayItemContext Builder::ArrayItemContext::Value(Node::Value value) {
		return BaseContext::Value(std::move(value));
	}

	Builder::DictItemContext Builder::ArrayItemContext::StartDict() {
		return BaseContext::StartDict();
	}

	Builder::ArrayItemContext Builder::ArrayItemContext::StartArray() {
		return BaseContext::StartArray();
	}

	Builder& Builder::ArrayItemContext::EndArray() {
		return BaseContext::EndArray();
	}

	//===============KeyItemContext=================

	Builder::DictItemContext Builder::KeyItemContext::Value(Node::Value value) {
		return BaseContext::Value(std::move(value));
	}

	Builder::DictItemContext Builder::KeyItemContext::StartDict() {
		return BaseContext::StartDict();
	}

	Builder::ArrayItemContext Builder::KeyItemContext::StartArray() {
		return BaseContext::StartArray();
	}
}