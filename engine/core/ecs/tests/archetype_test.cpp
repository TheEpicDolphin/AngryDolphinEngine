
#include <gtest/gtest.h>
#include <string>
#include <vector>
#include <iostream>
#include <algorithm>

#include <core/utils/gtest_helpers.h>
#include "../archetype.h"

struct A : public Component<A>
{
    std::string name;
};

struct B : public Component<B>
{
    std::string name;
};

struct C : public Component<C>
{
    std::string name;
};

template<typename T>
std::vector<T> _ConvertFrom(std::vector<T *>& input) 
{
    std::vector<T> output(input.size());
    std::transform(input.begin(), input.end(), output.begin(), [](T* t) { return *t; });
    return output;
}

TEST(archetype_test_suite, init_test)
{
    A::SetTypeId(1);
    B::SetTypeId(2);
    C::SetTypeId(3);
    Archetype archetype = Archetype::ArchetypeWithComponentTypes<A, B, C>();
    ASSERT_CONTAINERS_EQ(ordered_nodes, ordered_nodes.size(), expected_ordered_nodes, expected_ordered_nodes.size());
}
