
#include <gtest/gtest.h>
#include <string>
#include <vector>
#include <iostream>
#include <algorithm>

#include <core/utils/gtest_helpers.h>
#include "../archetype.h"

struct A
{
    std::string name;
};

struct B
{
    std::string name;
};

struct C
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

TEST(archetype_test_suite, adding_entity_test)
{
    Component<A>::SetTypeId(1);
    Component<B>::SetTypeId(1);
    Component<C>::SetTypeId(1);
    Archetype archetype = Archetype::ArchetypeWithComponentTypes<A, B, C>();
    archetype.AddEntity<B, C, A>(1, {"I'm B"}, {"I am C"}, {"My name is A"});
    
}
