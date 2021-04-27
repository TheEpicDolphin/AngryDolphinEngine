
#include <gtest/gtest.h>
#include <string>

#include "../set_trie.h"

TEST(testMath, myCubeTest)
{
    SetTrie<uint64_t, std::string> test_set_trie;
    test_set_trie.InsertValueForKeySet("node 1", { 3, 4, 7 });
    EXPECT_EQ(1000, 1000);
}