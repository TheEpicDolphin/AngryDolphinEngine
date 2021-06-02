
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
    const std::string a_name = "My name is A";
    const std::string b_name = "I'm B";
    const std::string c_name = "I am C";

    Component<A>::SetTypeId(1);
    Component<B>::SetTypeId(2);
    Component<C>::SetTypeId(3);
 
    Archetype *archetype = Archetype::ArchetypeWithComponentTypes<A, B, C>();
    archetype->AddEntity<B, C, A>(1, { b_name }, { c_name }, { a_name });
    B *b = archetype->GetComponentForEntity<B>(1);
    ASSERT_EQ(b->name, b_name);

    const std::vector<EntityID> entities = archetype->Entities();
    const std::vector<EntityID> expected_entities = { 1 };
    ASSERT_CONTAINERS_EQ(entities, entities.size(), expected_entities, expected_entities.size());
}
