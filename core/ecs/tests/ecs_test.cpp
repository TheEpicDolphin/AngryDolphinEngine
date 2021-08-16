
#include <gtest/gtest.h>
#include <string>
#include <vector>
#include <iostream>

#include <core/utils/gtest_helpers.h>
#include "../ecs.h"

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

TEST(ecs_test_suite, creating_entity_test)
{
    ECS ecs;
    EntityID entity_id = ecs.CreateEntity();
    ASSERT_EQ(entity_id, 1);
}

TEST(ecs_test_suite, adding_component_test)
{
    ECS ecs;
    EntityID entity_id = ecs.CreateEntity();
    ecs.AddComponent<A>(entity_id, { a_name_0 });
    A* a = ecs.GetComponent<A>(entity_id);
    ASSERT_EQ(a->name, a_name_0);

    ecs.AddComponent<B>(entity_id, { b_name_0 });

    ecs.EnumerateComponentsWithBlock<A, B>();
}