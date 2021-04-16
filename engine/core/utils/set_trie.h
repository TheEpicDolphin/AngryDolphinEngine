#pragma once

#include <iostream>
#include <map>
#include <vector>

template<typename TKey, typename TValue>
class SetTrie{
private:
	std::map<TKey, SetTrie<TKey, TValue>> subtries_;
public:
	TValue value;
	std::vector<TValue> FindSuperSets(std::vector<TKey> key_set){
		if(key_set.size() == 0){
			return [];
		}
		std::vector<TValue> super_set;
		TKey key_head = key_set[0];
		for(std::map<TKey, SetTrie<TKey, TValue>>::iterator iter = subtries_.begin(); iter != subtries_.end(); ++iter)
		{
			if(iter->first >= key_head){
				 return super_set;
			}
			else{
				super_set.append(FindSuperSets(key_set[1:]));
			}
		}
	}
}