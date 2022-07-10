
#include <gtest/gtest.h>
#include <string>
#include <vector>
#include <iostream>
#include <algorithm>

#include "../set_trie.h"
#include "../gtest_helpers.h"

template<typename T>
std::vector<T> _ConvertFrom(std::vector<T *>& input) 
{
    std::vector<T> output(input.size());
    std::transform(input.begin(), input.end(), output.begin(), [](T* t) { return *t; });
    return output;
}

TEST(set_trie_test_suite, insert_test)
{
    const std::string node_1 = "node 1";
    const std::string node_2 = "node 2";
    const std::string node_3 = "node 3";
    SetTrie<uint64_t, std::string> test_set_trie;
    test_set_trie.InsertValueForKeySet({ 3, 5, 6 }, node_1);
    test_set_trie.InsertValueForKeySet({ 3, 4, 7 }, node_2);
    std::string node_value;
    ASSERT_TRUE(test_set_trie.TryGetValueForKeySet({ 3, 5, 6 }, node_value));
    ASSERT_EQ(node_value, node_1);

    test_set_trie.InsertValueForKeySet({ 5, 9 }, node_3);
    ASSERT_TRUE(test_set_trie.TryGetValueForKeySet({ 3, 4, 7 }, node_value));
    ASSERT_EQ(node_value, node_2);

    const std::vector<std::string> ordered_nodes = _ConvertFrom(test_set_trie.GetValuesInOrder());
    const std::vector<std::string> expected_ordered_nodes = { node_2, node_1, node_3 };

    ASSERT_CONTAINERS_EQ(ordered_nodes, ordered_nodes.size(), expected_ordered_nodes, expected_ordered_nodes.size());
}

TEST(set_trie_test_suite, trie_structure_test)
{
    const std::string node_1 = "node 1";
    const std::string node_2 = "node 2";
    const std::string node_3 = "node 3";
    const std::string node_4 = "node 4";
    const std::string node_5 = "node 5";
    const std::string node_6 = "node 6";
    const std::string node_7 = "node 7";
    SetTrie<uint64_t, std::string> test_set_trie;
    test_set_trie.InsertValueForKeySet({ 3, 5, 6 }, node_1);
    test_set_trie.InsertValueForKeySet({ 3, 4, 7 }, node_2);
    test_set_trie.InsertValueForKeySet({ 1, 3 }, node_3);
    test_set_trie.InsertValueForKeySet({ 4, 5, 6, 7 }, node_4);
    test_set_trie.InsertValueForKeySet({ 2, 4, 6, 10 }, node_5);
    test_set_trie.InsertValueForKeySet({ 2 }, node_6);
    test_set_trie.InsertValueForKeySet({ 1, 2 }, node_7);

    const std::vector<std::string> ordered_nodes = _ConvertFrom(test_set_trie.GetValuesInOrder());
    const std::vector<std::string> expected_ordered_nodes = { node_7, node_3, node_6, node_5, node_2, node_1, node_4 };

    ASSERT_CONTAINERS_EQ(ordered_nodes, ordered_nodes.size(), expected_ordered_nodes, expected_ordered_nodes.size());
}

TEST(set_trie_test_suite, removal_test)
{
    const std::string node_1 = "node 1";
    const std::string node_2 = "node 2";
    const std::string node_3 = "node 3";
    const std::string node_4 = "node 4";

    SetTrie<uint64_t, std::string> test_set_trie;
    test_set_trie.InsertValueForKeySet({ 3, 4, 6 }, node_1);
    test_set_trie.InsertValueForKeySet({ 3, 4, 7 }, node_2);
    test_set_trie.InsertValueForKeySet({ 1, 3 }, node_3);
    test_set_trie.InsertValueForKeySet({ 4, 5, 6, 7 }, node_4);

    test_set_trie.RemoveValueForKeySet({ 3, 4, 7 });
    test_set_trie.RemoveValueForKeySet({ 1, 3 });

    const std::vector<std::string> ordered_nodes = _ConvertFrom(test_set_trie.GetValuesInOrder());
    const std::vector<std::string> expected_ordered_nodes = { node_1, node_4 };

    ASSERT_CONTAINERS_EQ(ordered_nodes, ordered_nodes.size(), expected_ordered_nodes, expected_ordered_nodes.size());
}

TEST(set_trie_test_suite, value_modification_test)
{
    const std::string node_1 = "node 1";
    const std::string node_2 = "node 2";
    const std::string node_3 = "node 3";

    SetTrie<uint64_t, std::string> test_set_trie;
    test_set_trie.InsertValueForKeySet({ 3, 4, 6 }, node_1);
    test_set_trie.InsertValueForKeySet({ 3, 4, 7 }, node_2);
    test_set_trie.InsertValueForKeySet({ 1, 3 }, node_3);

    std::string node_val_2;
    ASSERT_TRUE(test_set_trie.TryGetValueForKeySet({ 3, 4, 7 }, node_val_2));
    node_val_2[1] = 'a';
    std::string node_val_2_modified;
    ASSERT_TRUE(test_set_trie.TryGetValueForKeySet({ 3, 4, 7 }, node_val_2_modified));
    ASSERT_EQ(node_val_2_modified, "nade 2");

    std::string node_val_3;
    ASSERT_TRUE(test_set_trie.TryGetValueForKeySet({ 1, 3 }, node_val_3));
    node_val_3[2] = 's';
    std::string node_val_3_modified;
    ASSERT_TRUE(test_set_trie.TryGetValueForKeySet({ 1, 3 }, node_val_3_modified));
    ASSERT_EQ(node_val_3_modified, "nose 3");
}
