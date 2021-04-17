#pragma once

#include <iostream>
#include <map>
#include <vector>
#include <stack>

template<typename TKey, typename TValue>
struct SetTrieNode {
	TKey key;
	TValue value;
	std::map<TKey, SetTrieNode<TKey, TValue>> children;
};

template<typename TKey, typename TValue, class Compare = std::less<TKey>>
class SetTrie {
private:
	SetTrieNode<TKey, TValue> root_node_;

public:
	SetTrie() {
		
	}

	std::vector<TValue> FindSuperSets(std::vector<TKey> key_set) {
		std::vector<TValue> supersets;
		std::size_t index = 0;
		std::stack<std::pair<SetTrieNode<TKey, TValue>, std::map<TKey, SetTrie<TKey, TValue>>::iterator>> stack;
		stack.push(std::make_pair(root_node_, root_node_.children.begin()));
		while (!stack.empty()) {
			SetTrieNode<TKey, TValue> current_node = stack.top().first;
			std::map<TKey, SetTrieNode<TKey, TValue>>::iterator children_iter = stack.top().second;
			if (current_node.key < key_set[index] || children_iter == current_node.children.end()) {
				stack.pop();
				if (index > 0 && current_node.key == key_set[index - 1]) {
					index--;
				}
			}
			else {
				SetTrieNode<TKey, TValue> next_node = children_iter->second;
				stack.push(std::make_pair(next_node, next_node.children.begin()));
				if (index < key_set.size() && next_node.key == key_set[index]) {
					index++;
				}
				if (index == key_set.size() && next_node.value) {
					supersets.push_back(next_node.value);
				}

				std::advance(stack.top().second);
			}
		}

		return supersets;
	}

	void InsertValueForKeySet(TValue value, std::vector<TKey> key_set) {
		SetTrieNode<TKey, TValue> current_node = root_node_;
		std::size_t index = 0;
		while (index < key_set.size()) {
			std::map<TKey, SetTrieNode<TKey, TValue>>::iterator iter = current_node.children.find(key_set[index]);
			if (iter == current_node.children.end()) {
				break;
			}
			index++;
			current_node = iter->second;
		}

		if (index == key_set.size() && current_node.value) {
			throw std::runtime_error("Trying to insert value for keyset that is already in Set Trie");
			return;
		}

		while (index < key_set.size()) {
			SetTrieNode<TKey, TValue> new_node;
			new_node.key = key_set[index];
			current_node.children.insert({ new_node.key, new_node });
			current_node = new_node;
			index++;
		}
	}

	void RemoveValueForKeySet(std::vector<TKey> key_set) {
		SetTrieNode<TKey, TValue> current_node = root_node_;
		std::size_t index = 0;
		std::stack<SetTrieNode<TKey, TValue>::iterator> stack;
		while (index < key_set.size()) {
			std::map<TKey, SetTrieNode<TKey, TValue>>::iterator iter = current_node.children.find(key_set[index]);
			if (iter == current_node.children.end()) {
				throw std::runtime_error("Keyset cannot be found in Set Trie");
				return;
			}
			index++;
			current_node = iter->second;
			stack.push(current_node);
		}

		current_node.value = NULL;
		// Remove value-less nodes
		while (stack.top().value == NULL) {
			current_node.children.erase();
		}
	}
};