
#include <gtest/gtest.h>
#include <string>
#include <vector>
#include <iostream>

#include "../set_trie.h"

TEST(set_trie_test_suite, insert_test)
{
    const std::string node_1 = "node 1";
    const std::string node_2 = "node 2";
    const std::string node_3 = "node 3";
    SetTrie<uint64_t, std::string> test_set_trie;
    test_set_trie.InsertValueForKeySet(node_1, { 3, 5, 6 } );
    test_set_trie.InsertValueForKeySet(node_2, { 3, 4, 7 } );
    std::string node_value = test_set_trie.ValueForKeySet({ 3, 5, 6 });
    EXPECT_EQ(node_value, node_1);

    test_set_trie.InsertValueForKeySet(node_3, { 5, 9 });
    node_value = test_set_trie.ValueForKeySet({3, 4, 7});
    EXPECT_EQ(node_value, node_2);

    const std::vector<std::string> ordered_nodes = test_set_trie.GetValuesInOrder();
    const std::vector<std::string> expected_ordered_nodes = { node_2, node_1, node_3 };
    EXPECT_EQ(ordered_nodes.size(), expected_ordered_nodes.size());
    std::cout << ordered_nodes.size() << std::endl;
    for (std::size_t i = 0; i < ordered_nodes.size(); i++) {
        EXPECT_EQ(ordered_nodes[i], expected_ordered_nodes[i]);
        std::cout << ordered_nodes[i] << std::endl;
    }
}
