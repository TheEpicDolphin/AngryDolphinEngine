
#include <gtest/gtest.h>
#include <string>
#include <vector>
#include <iostream>

#include <core/utils/gtest_helpers.h>
#include <core/utils/type_info.h>
#include "../archetype.h"

using namespace ecs;

static const std::string a_name_0 = "A0";
static const std::string a_name_1 = "A1";
static const std::string a_name_2 = "A2";
static const std::string a_name_3 = "A3";
static const std::string a_name_4 = "A4";

static const std::string b_name_0 = "B0";
static const std::string b_name_1 = "B1";
static const std::string b_name_2 = "B2";
static const std::string b_name_3 = "B3";
static const std::string b_name_4 = "B4";

static const std::string c_name_0 = "C0";
static const std::string c_name_1 = "C1";
static const std::string c_name_2 = "C2";
static const std::string c_name_3 = "C3";
static const std::string c_name_4 = "C4";

static const std::string d_name_0 = "D0";
static const std::string d_name_1 = "D1";
static const std::string d_name_2 = "D2";
static const std::string d_name_3 = "D3";
static const std::string d_name_4 = "D4";

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
    TypeInfo component_type_info;
    component_type_info.GetTypeId<A>();
    component_type_info.GetTypeId<B>();
    component_type_info.GetTypeId<C>();
 
    Archetype *archetype = Archetype::ArchetypeWithComponentTypes<A, B, C>(&component_type_info);
    archetype->AddEntity<B, C, A>({ 0, 1 }, { b_name_0 }, { c_name_0 }, { a_name_0 });
    B *b = archetype->GetComponentForEntity<B>({ 0, 1 });
    ASSERT_EQ(b->name, b_name_0);

    const std::vector<EntityID> entities = archetype->Entities();
    const std::vector<EntityID> expected_entities = { { 0, 1 } };
    ASSERT_CONTAINERS_EQ(entities, entities.size(), expected_entities, expected_entities.size());
}

TEST(archetype_test_suite, removing_entity_test)
{
    TypeInfo component_type_info;
    component_type_info.GetTypeId<A>();
    component_type_info.GetTypeId<B>();
    component_type_info.GetTypeId<C>();

    Archetype* archetype = Archetype::ArchetypeWithComponentTypes<A, B, C>(&component_type_info);
    archetype->AddEntity<B, C, A>({ 0, 1 }, { b_name_0 }, { c_name_0 }, { a_name_0 });
    archetype->AddEntity<B, C, A>({ 0, 2 }, { b_name_1 }, { c_name_1 }, { a_name_1 });

    std::vector<EntityID> expected_entities = { {0, 1}, {0, 2} };
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

    archetype->RemoveEntity({ 0, 1 });
    expected_entities = { {0, 2} };
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

    TypeInfo component_type_info;
    component_type_info.GetTypeId<A>();
    component_type_info.GetTypeId<B>();
    component_type_info.GetTypeId<C>();
    component_type_info.GetTypeId<D>();

    Archetype* archetype_abc = Archetype::ArchetypeWithComponentTypes<A, B, C>(&component_type_info);
    archetype_abc->AddEntity<B, C, A>({ 0, 1 }, { b_name_0 }, { c_name_0 }, { a_name_0 });
    archetype_abc->AddEntity<B, C, A>({ 0, 2 }, { b_name_1 }, { c_name_1 }, { a_name_1 });

    Archetype* archetype_abcd = archetype_abc->EmptyWithAddedComponentType<D>();
    archetype_abcd->AddEntity<D, B, C, A>({ 0, 3 }, { d_name_2 }, { b_name_2 }, { c_name_2 }, { a_name_2 });

    std::vector<EntityID> expected_entities = { { 0, 3 } };
    std::vector<A> expected_a_components = { { a_name_2 } };
    std::vector<B> expected_b_components = { { b_name_2 } };
    std::vector<C> expected_c_components = { { c_name_2 } };
    std::vector<D> expected_d_components = { { d_name_2 } };
    
    std::size_t index = 0;
    std::function<void(EntityID, A&, B&, C&, D&)> test_block =
        [&](EntityID entity_id, A& a, B& b, C& c, D& d) {
        ASSERT_EQ(expected_entities[index], entity_id);
        ASSERT_EQ(expected_a_components[index].name, a.name);
        ASSERT_EQ(expected_b_components[index].name, b.name);
        ASSERT_EQ(expected_c_components[index].name, c.name);
        ASSERT_EQ(expected_d_components[index].name, d.name);
        index++;
    };

    archetype_abcd->EnumerateComponentsWithBlock<A, B, C, D>(test_block);
}

TEST(archetype_test_suite, creating_archetype_with_removed_component_type)
{
    TypeInfo component_type_info;
    component_type_info.GetTypeId<A>();
    component_type_info.GetTypeId<B>();
    component_type_info.GetTypeId<C>();
    component_type_info.GetTypeId<D>();

    Archetype* archetype_abcd = Archetype::ArchetypeWithComponentTypes<A, B, C, D>(&component_type_info);
    archetype_abcd->AddEntity<D, B, C, A>({ 0, 1 }, { d_name_0 }, { b_name_0 }, { c_name_0 }, { a_name_0 });
    archetype_abcd->AddEntity<D, B, C, A>({ 0, 2 }, { d_name_1 }, { b_name_1 }, { c_name_1 }, { a_name_1 });

    Archetype* archetype_acd = archetype_abcd->EmptyWithRemovedComponentType<B>();
    archetype_acd->AddEntity<D, C, A>({ 0, 3 }, { d_name_2 }, { c_name_2 }, { a_name_2 });

    std::vector<EntityID> expected_entities = { { 0, 3 } };
    std::vector<A> expected_a_components = { { a_name_2 } };
    std::vector<C> expected_c_components = { { c_name_2 } };
    std::vector<D> expected_d_components = { { d_name_2 } };

    std::size_t index = 0;
    std::function<void(EntityID, A&, C&, D&)> test_block =
        [&](EntityID entity_id, A& a, C& c, D& d) {
        ASSERT_EQ(expected_entities[index], entity_id);
        ASSERT_EQ(expected_a_components[index].name, a.name);
        ASSERT_EQ(expected_c_components[index].name, c.name);
        ASSERT_EQ(expected_d_components[index].name, d.name);
        index++;
    };

    archetype_acd->EnumerateComponentsWithBlock<A, C, D>(test_block);
    B *b = archetype_acd->GetComponentForEntity<B>({ 0, 3 });
    ASSERT_EQ(b, nullptr);
}

TEST(archetype_test_suite, moving_entity_to_super_archetype)
{
    TypeInfo component_type_info;
    component_type_info.GetTypeId<A>();
    component_type_info.GetTypeId<B>();
    component_type_info.GetTypeId<C>();
    component_type_info.GetTypeId<D>();

    Archetype* archetype_abc = Archetype::ArchetypeWithComponentTypes<A, B, C>(&component_type_info);
    archetype_abc->AddEntity<A, B, C>({ 0, 1 }, { a_name_0 }, { b_name_0 }, { c_name_0 });
    archetype_abc->AddEntity<A, B, C>({ 0, 2 }, { a_name_1 }, { b_name_1 }, { c_name_1 });

    Archetype* archetype_abcd = Archetype::ArchetypeWithComponentTypes<A, B, C, D>(&component_type_info);
    archetype_abcd->AddEntity<D, B, C, A>({ 0, 3 }, { d_name_2 }, { b_name_2 }, { c_name_2 }, { a_name_2 });
    archetype_abcd->AddEntity<D, B, C, A>({ 0, 4 }, { d_name_3 }, { b_name_3 }, { c_name_3 }, { a_name_3 });

    archetype_abc->MoveEntityToSuperArchetype<D>({ 0, 1 }, *archetype_abcd, { d_name_0 });
    std::vector<EntityID> expected_entities = { { 0, 2 } };
    std::vector<A> expected_a_components = { { a_name_1 } };
    std::vector<A> expected_b_components = { { b_name_1 } };
    std::vector<C> expected_c_components = { { c_name_1 } };
    std::size_t index = 0;
    std::function<void(EntityID, A&, B&, C&)> abc_test_block =
        [&](EntityID entity_id, A& a, B& b, C& c) {
        ASSERT_EQ(expected_entities[index], entity_id);
        ASSERT_EQ(expected_a_components[index].name, a.name);
        ASSERT_EQ(expected_b_components[index].name, b.name);
        ASSERT_EQ(expected_c_components[index].name, c.name);
        index++;
    };
    archetype_abc->EnumerateComponentsWithBlock<A, B, C>(abc_test_block);

    expected_entities = { { 0, 3 }, { 0, 4 }, { 0, 1 } };
    expected_a_components = { { a_name_2 }, { a_name_3 }, { a_name_0 } };
    expected_b_components = { { b_name_2 }, { b_name_3 }, { b_name_0 } };
    expected_c_components = { { c_name_2 }, { c_name_3 }, { c_name_0 } };
    std::vector<D> expected_d_components = { { d_name_2 }, { d_name_3 }, { d_name_0 } };
    index = 0;
    std::function<void(EntityID, A&, B&, C&, D&)> abcd_test_block =
        [&](EntityID entity_id, A& a, B& b, C& c, D& d) {
        ASSERT_EQ(expected_entities[index], entity_id);
        ASSERT_EQ(expected_a_components[index].name, a.name);
        ASSERT_EQ(expected_b_components[index].name, b.name);
        ASSERT_EQ(expected_c_components[index].name, c.name);
        ASSERT_EQ(expected_d_components[index].name, d.name);
        index++;
    };
    archetype_abcd->EnumerateComponentsWithBlock<A, B, C, D>(abcd_test_block);
}

TEST(archetype_test_suite, moving_entity_to_sub_archetype)
{
    TypeInfo component_type_info;
    component_type_info.GetTypeId<A>();
    component_type_info.GetTypeId<B>();
    component_type_info.GetTypeId<C>();
    component_type_info.GetTypeId<D>();

    Archetype* archetype_abc = Archetype::ArchetypeWithComponentTypes<A, B, C>(&component_type_info);
    archetype_abc->AddEntity<A, B, C>({ 0, 1 }, { a_name_0 }, { b_name_0 }, { c_name_0 });
    archetype_abc->AddEntity<A, B, C>({ 0, 2 }, { a_name_1 }, { b_name_1 }, { c_name_1 });

    Archetype* archetype_abcd = Archetype::ArchetypeWithComponentTypes<A, B, C, D>(&component_type_info);
    archetype_abcd->AddEntity<D, B, C, A>({ 0, 3 }, { d_name_2 }, { b_name_2 }, { c_name_2 }, { a_name_2 });
    archetype_abcd->AddEntity<D, B, C, A>({ 0, 4 }, { d_name_3 }, { b_name_3 }, { c_name_3 }, { a_name_3 });
    archetype_abcd->AddEntity<D, B, C, A>({ 0, 5 }, { d_name_4 }, { b_name_4 }, { c_name_4 }, { a_name_4 });

    archetype_abcd->MoveEntityToSubArchetype<D>({ 0, 3 }, *archetype_abc);
    std::vector<EntityID> expected_entities = { { 0, 1 }, { 0, 2 }, { 0, 3 } };
    std::vector<A> expected_a_components = { { a_name_0 }, { a_name_1 }, { a_name_2 } };
    std::vector<A> expected_b_components = { { b_name_0 }, { b_name_1 }, { b_name_2 } };
    std::vector<C> expected_c_components = { { c_name_0 }, { c_name_1 }, { c_name_2 } };
    std::size_t index = 0;
    std::function<void(EntityID, A&, B&, C&)> abc_test_block =
        [&](EntityID entity_id, A& a, B& b, C& c) {
        ASSERT_EQ(expected_entities[index], entity_id);
        ASSERT_EQ(expected_a_components[index].name, a.name);
        ASSERT_EQ(expected_b_components[index].name, b.name);
        ASSERT_EQ(expected_c_components[index].name, c.name);
        index++;
    };
    archetype_abc->EnumerateComponentsWithBlock<A, B, C>(abc_test_block);

    expected_entities = { { 0, 5 }, { 0, 4 } };
    expected_a_components = { { a_name_4 }, { a_name_3 } };
    expected_b_components = { { b_name_4 }, { b_name_3 } };
    expected_c_components = { { c_name_4 }, { c_name_3 } };
    std::vector<D> expected_d_components = { { d_name_4 }, { d_name_3 } };
    index = 0;
    std::function<void(EntityID, A&, B&, C&, D&)> abcd_test_block =
        [&](EntityID entity_id, A& a, B& b, C& c, D& d) {
        ASSERT_EQ(expected_entities[index], entity_id);
        ASSERT_EQ(expected_a_components[index].name, a.name);
        ASSERT_EQ(expected_b_components[index].name, b.name);
        ASSERT_EQ(expected_c_components[index].name, c.name);
        ASSERT_EQ(expected_d_components[index].name, d.name);
        index++;
    };
    archetype_abcd->EnumerateComponentsWithBlock<A, B, C, D>(abcd_test_block);
}