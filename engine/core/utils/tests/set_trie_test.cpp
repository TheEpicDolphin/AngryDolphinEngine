
#include <gtest/gtest.h>
#include <string>

#include "../set_trie.h"

TEST(set_trie_test_suite, insert_test)
{
    const std::string node_1 = "node 1";
    const std::string node_2 = "node 2";
    SetTrie<uint64_t, std::string> test_set_trie;
    test_set_trie.InsertValueForKeySet(node_1, { 3, 4, 7 });
    EXPECT_EQ(node_1, node_2);
    //test_set_trie.InsertValueForKeySet(node_2, { 3, 5, 6 });
    //std::string node_value = test_set_trie.ValueForKeySet({ 3, 5, 6 });
    //EXPECT_EQ(node_value, node_2);
}