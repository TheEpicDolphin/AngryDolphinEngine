#pragma once

#include <map>
#include <stack>
#include <vector>

template<typename TKey, typename TValue>
class SetTrie {
private:
	struct SetTrieNode {
		TKey key;
		std::unique_ptr<TValue> value_ptr;
		//std::shared_ptr<TValue> value_ptr;
		std::map<TKey, SetTrieNode> children;
	};

	SetTrieNode root_node_;

public:
	SetTrie() {
		root_node_.key = 0;
		root_node_.value_ptr.reset();
	}

	std::vector<TValue> FindSuperSets(std::vector<TKey> key_set) {
		std::vector<TValue> supersets;
		std::size_t index = 0;
		std::stack<std::pair<SetTrieNode, std::map<TKey, SetTrieNode>::iterator>> stack;
		stack.push(std::make_pair(root_node_, root_node_.children.begin()));
		while (!stack.empty()) {
			SetTrieNode current_node = stack.top().first;
			std::map<TKey, SetTrieNode>::iterator children_iter = stack.top().second;
			if (current_node.key < key_set[index] || children_iter == current_node.children.end()) {
				stack.pop();
				if (index > 0 && current_node.key == key_set[index - 1]) {
					index--;
				}
			}
			else {
				SetTrieNode next_node = children_iter->second;
				stack.push(std::make_pair(next_node, next_node.children.begin()));
				if (index < key_set.size() && next_node.key == key_set[index]) {
					index++;
				}
				if (index == key_set.size() && next_node.value_ptr) {
					supersets.push_back(*next_node.value_ptr.get());
				}

				std::advance(stack.top().second, 1);
			}
		}

		return supersets;
	}

	void InsertValueForKeySet(TValue value, std::vector<TKey> key_set) {
		SetTrieNode *current_node = &root_node_;
		std::size_t index = 0;
		while (index < key_set.size()) {
			std::map<TKey, SetTrieNode>::iterator children_iter = current_node->children.find(key_set[index]);
			if (children_iter == current_node->children.end()) {
				break;
			}
			index++;
			current_node = &children_iter->second;
		}

		if (index == key_set.size() && current_node->value_ptr) {
			throw std::runtime_error("Trying to insert value for keyset that is already in Set Trie");
			return;
		}

		while (index < key_set.size()) {
			SetTrieNode new_node;
			new_node.key = key_set[index];
			new_node.value_ptr = std::make_unique<TValue>(value);
			//new_node.value_ptr = std::make_shared<TValue>(value);
			std::pair<std::map<TKey, SetTrieNode>::iterator, bool> result = current_node->children.insert(std::make_pair(new_node.key, new_node));
			current_node = &result.first->second;
			index++;
		}
	}

	void RemoveValueForKeySet(std::vector<TKey> key_set) {
		std::size_t index = 0;
		std::stack<SetTrieNode*> stack;
		stack.push(&root_node_);
		while (index < key_set.size()) {
			std::map<TKey, SetTrieNode>::iterator children_iter = stack.top()->children.find(key_set[index]);
			if (children_iter == stack.top()->children.end()) {
				throw std::runtime_error("Keyset cannot be found in Set Trie");
				return;
			}
			index++;
			stack.push(&children_iter->second);
		}

		stack.top()->value_ptr.reset();

		// If top node has no children, then we should traverse ancestors to delete any unnecessary nodes
		while (!stack.top()->value_ptr && stack.top()->children.empty()) {
			TKey childKey = stack.top()->key;
			stack.pop();
			stack.top()->children.erase(childKey);
		}
	}

	TValue FindValueForKeySet(std::vector<TKey> key_set) {
		SetTrieNode *current_node = &root_node_;
		std::size_t index = 0;
		while (index < key_set.size()) {
			std::map<TKey, SetTrieNode>::iterator children_iter = current_node->children.find(key_set[index]);
			if (children_iter == current_node->children.end()) {
				return NULL;
			}
			index++;
			current_node = &children_iter->second;
		}
		return *current_node->value_ptr.get();
	}
};