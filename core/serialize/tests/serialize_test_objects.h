#pragma once

#include <vector>
#include <string>

#include "../serializable.h"

class SimpleClass : public ISerializable
{
public:
    SimpleClass() {};

    SimpleClass(int i, float f, std::string s)
    {
        i_ = i;
        f_ = f;
        s_ = s;
    }

    void SerializeHumanReadable(Archive& archive, rapidxml::xml_node<>& xml_node) override
    {
        archive.SerializeHumanReadable<int&, float&, std::string&>(
            xml_node,
            { "i", i_ },
            { "f", f_ },
            { "s", s_ }
        );
    }

    void DeserializeHumanReadable(Archive& archive, rapidxml::xml_node<>& xml_node) override {};

private:
    int i_;
    float f_;
    std::string s_;
};

class ParentClass : public ISerializable
{
public:
    ParentClass(std::vector<int> vec, SimpleClass sc)
    {
        vec_ = vec;
        sc_ = sc;
    }

    void SerializeHumanReadable(Archive& archive, rapidxml::xml_node<>& xml_node) override
    {
        archive.SerializeHumanReadable<std::vector<int>&, SimpleClass&>(
            xml_node,
            { "vec", vec_ },
            { "sc", sc_ }
        );
    }

    void DeserializeHumanReadable(Archive& archive, rapidxml::xml_node<>& xml_node) override {};

private:
    std::vector<int> vec_;
    SimpleClass sc_;
};

class ParentClassWithHeapPointer : public ISerializable
{
public:
    ParentClassWithHeapPointer(std::uint64_t u)
    {
        sc_ptr_ = new SimpleClass(50, -3.14f, "I was allocated on the heap!");
        u_ = u;
    }

    void SerializeHumanReadable(Archive& archive, rapidxml::xml_node<>& xml_node) override
    {
        archive.SerializeHumanReadable<SimpleClass*&, std::uint64_t&>(
            xml_node,
            { "simple_class_ptr", sc_ptr_ },
            { "u", u_ }
        );
    }

    void DeserializeHumanReadable(Archive& archive, rapidxml::xml_node<>& xml_node) override {};

private:
    SimpleClass* sc_ptr_;
    std::uint64_t u_;
};