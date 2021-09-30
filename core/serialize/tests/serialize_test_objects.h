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

    void SerializeHumanReadable(Archive& archive, std::ostream& ostream) override
    {
        archive.SerializeHumanReadable<int&, float&, std::string&>(
            ostream,
            { "i", i_ },
            { "f", f_ },
            { "s", s_ }
        );
    }

    void DeserializeHumanReadable(Archive& archive, std::ostream& ostream) override {};

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

    void SerializeHumanReadable(Archive& archive, std::ostream& ostream) override
    {
        archive.SerializeHumanReadable<std::vector<int>&, SimpleClass&>(
            ostream,
            { "vec", vec_ },
            { "sc", sc_ }
        );
    }

    void DeserializeHumanReadable(Archive& archive, std::ostream& ostream) override {};

private:
    std::vector<int> vec_;
    SimpleClass sc_;
};