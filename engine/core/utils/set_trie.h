#pragma once

#include <map>
#include <stack>
#include <vector>
#include <iostream>

template<typename TKey, typename TValue>
class SetTrie {
private:
	struct SetTrieNode {
		TKey key;
		TValue value;
		bool has_value;
		std::map<TKey, SetTrieNode> children;
	};

	SetTrieNode root_node_;

public:
	SetTrie() {
		root_node_.key = 0;
		root_node_.has_value = false;
	}

	std::vector<TValue *> GetValuesInOrder()
	{
		return FindSuperKeySetValues({});
	}

	/*
		  0
		 / \
		1   3
	   / \
	  2   3

	*/

	std::vector<TValue *> FindSuperKeySetValues(std::vector<TKey> key_set) {
		std::vector<TValue *> supersets;
		std::size_t index = 0;
		std::stack<std::pair<SetTrieNode *, std::map<TKey, SetTrieNode>::iterator>> stack;
		stack.push(std::make_pair(&root_node_, root_node_.children.begin()));
		while (!stack.empty()) {
			SetTrieNode *current_node = stack.top().first;
			std::map<TKey, SetTrieNode>::iterator children_iter = stack.top().second;
			// Check if there are no more children OR if the next child's key is larger than the next key we are looking for.
			// In both cases, we should stop traversing the children of current_node.
			if (children_iter == current_node->children.end() || (index < key_set.size() && children_iter->second.key > key_set[index])) {
				stack.pop();
				if (index > 0 && current_node->key == key_set[index - 1]) {
					index--;
				}
			}
			else {
				SetTrieNode *next_node = &children_iter->second;
				std::advance(stack.top().second, 1);
				stack.push(std::make_pair(next_node, next_node->children.begin()));
				if (index < key_set.size() && next_node->key == key_set[index]) {
					index++;
				}
				if (index == key_set.size() && next_node->has_value) {
					supersets.push_back(&next_node->value);
				}	
			}
		}
		return supersets;
	}

	TValue* InsertValueForKeySet(TValue value, std::vector<TKey> key_set) {
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

		if (index == key_set.size() && current_node->has_value) {
			throw std::runtime_error("Trying to insert value for keyset that is already in Set Trie");
			return nullptr;
		}

		while (index < key_set.size()) {
			SetTrieNode new_node;
			new_node.key = key_set[index];
			new_node.has_value = false;
			std::pair<std::map<TKey, SetTrieNode>::iterator, bool> result = current_node->children.insert(std::make_pair(new_node.key, new_node));
			current_node = &result.first->second;
			index++;
			
		}
		current_node->value = value;
		current_node->has_value = true;
		return &current_node->value;
	}

	void RemoveValueForKeySet(std::vector<TKey> key_set) {
		std::size_t index = 0;
		std::stack<SetTrieNode *> stack;
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

		stack.top()->has_value = false;

		// If top node has no children, then we should traverse ancestors to delete any unnecessary nodes
		while (!stack.top()->has_value && stack.top()->children.empty()) {
			TKey childKey = stack.top()->key;
			stack.pop();
			stack.top()->children.erase(childKey);
		}
	}

	TValue* ValueForKeySet(std::vector<TKey> key_set) {
		SetTrieNode *current_node = &root_node_;
		std::size_t index = 0;
		while (index < key_set.size()) {
			std::map<TKey, SetTrieNode>::iterator children_iter = current_node->children.find(key_set[index]);
			if (children_iter == current_node->children.end()) {
				// keyset was not found in set trie.
				return nullptr;
			}
			index++;
			current_node = &children_iter->second;
		}
		if (!current_node->has_value) {
			// keyset exists in set trie but the corresponding value is not set.
			return nullptr;
		}
		return &current_node->value;
	}
};