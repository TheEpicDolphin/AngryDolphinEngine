
#include <gtest/gtest.h>
#include <string>
#include <vector>
#include <iostream>
#include <algorithm>

#include <core/utils/gtest_helpers.h>
#include "../archetype.h"

static const std::string a_name_0 = "My name is A";
static const std::string b_name_0 = "I'm B";
static const std::string c_name_0 = "I am C";

static const std::string a_name_1 = "Heyo it's A";
static const std::string b_name_1 = "Me llamo B";
static const std::string c_name_1 = "Yo soy C";

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

TEST(archetype_test_suite, adding_entity_test)
{
    Component<A>::SetTypeId(1);
    Component<B>::SetTypeId(2);
    Component<C>::SetTypeId(3);
 
    Archetype *archetype = Archetype::ArchetypeWithComponentTypes<A, B, C>();
    archetype->AddEntity<B, C, A>(1, { b_name_0 }, { c_name_0 }, { a_name_0 });
    B *b = archetype->GetComponentForEntity<B>(1);
    ASSERT_EQ(b->name, b_name_0);

    const std::vector<EntityID> entities = archetype->Entities();
    const std::vector<EntityID> expected_entities = { 1 };
    ASSERT_CONTAINERS_EQ(entities, entities.size(), expected_entities, expected_entities.size());
}

TEST(archetype_test_suite, removing_entity_test)
{
    Component<A>::SetTypeId(1);
    Component<B>::SetTypeId(2);
    Component<C>::SetTypeId(3);

    Archetype* archetype = Archetype::ArchetypeWithComponentTypes<A, B, C>();
    archetype->AddEntity<B, C, A>(1, { b_name_0 }, { c_name_0 }, { a_name_0 });
    archetype->AddEntity<B, C, A>(2, { b_name_1 }, { c_name_1 }, { a_name_1 });

    std::vector<EntityID> expected_entities = { 1, 2 };
    std::vector<C> expected_c_components = { { c_name_0 }, { c_name_1 } };
    std::vector<B> expected_b_components = { { b_name_0 }, { b_name_1 } };
    std::vector<A> expected_a_components = { { a_name_0 }, { a_name_1 } };
    std::size_t index = 0;
    std::function<void(EntityID, C&, B&, A&)> test_block = 
    [&](EntityID entity_id, C& c, B& b, A& a) {
        ASSERT_EQ(expected_entities[index], entity_id);
        ASSERT_EQ(expected_c_components[index].name, c.name);
        ASSERT_EQ(expected_b_components[index].name, b.name);
        ASSERT_EQ(expected_a_components[index].name, a.name);
        index++;
    };

    archetype->EnumerateComponentsWithBlock<C, B, A>(test_block);

    archetype->RemoveEntity(1);
    expected_entities = { 2 };
    expected_c_components = { { c_name_1 } };
    expected_b_components = { { b_name_1 } };
    expected_a_components = { { a_name_1 } };
    index = 0;
    archetype->EnumerateComponentsWithBlock<C, B, A>(test_block);
}

struct D 
{
    std::string name;
};

TEST(archetype_test_suite, creating_archetype_with_added_component_type)
{
    Component<A>::SetTypeId(1);
    Component<B>::SetTypeId(2);
    Component<C>::SetTypeId(3);

    Archetype* archetype_abc = Archetype::ArchetypeWithComponentTypes<A, B, C>();
    archetype_abc->AddEntity<B, C, A>(1, { b_name_0 }, { c_name_0 }, { a_name_0 });
    archetype_abc->AddEntity<B, C, A>(2, { b_name_1 }, { c_name_1 }, { a_name_1 });

    Archetype* archetype_abcd = archetype_abc->EmptyWithAddedComponentType<D>();

    std::vector<EntityID> expected_entities = { 1, 2 };
    std::vector<C> expected_c_components = { { c_name_0 }, { c_name_1 } };
    std::vector<B> expected_b_components = { { b_name_0 }, { b_name_1 } };
    std::vector<A> expected_a_components = { { a_name_0 }, { a_name_1 } };
    std::size_t index = 0;
    std::function<void(EntityID, C&, B&, A&)> test_block =
        [&](EntityID entity_id, C& c, B& b, A& a) {
        ASSERT_EQ(expected_entities[index], entity_id);
        ASSERT_EQ(expected_c_components[index].name, c.name);
        ASSERT_EQ(expected_b_components[index].name, b.name);
        ASSERT_EQ(expected_a_components[index].name, a.name);
        index++;
    };

    archetype->EnumerateComponentsWithBlock<C, B, A>(test_block);

    archetype->RemoveEntity(1);
    expected_entities = { 2 };
    expected_c_components = { { c_name_1 } };
    expected_b_components = { { b_name_1 } };
    expected_a_components = { { a_name_1 } };
    index = 0;
    archetype->EnumerateComponentsWithBlock<C, B, A>(test_block);
}
