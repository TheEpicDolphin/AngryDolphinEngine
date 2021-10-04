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

    std::vector<ArchiveNodeBase*> SerializeHumanReadable(Archive& archive) override
    {
        return
        archive.RegisterMembers<int&, float&, std::string&>(
            { "i", i_ },
            { "f", f_ },
            { "s", s_ }
        );
    }

    std::vector<ArchiveNodeBase*> DeserializeHumanReadable(Archive& archive) override { return {}; };

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

    std::vector<ArchiveNodeBase*> SerializeHumanReadable(Archive& archive) override
    {
        return
        archive.RegisterMembers<std::vector<int>&, SimpleClass&>(
            { "vec", vec_ },
            { "sc", sc_ }
        );
    }

    std::vector<ArchiveNodeBase*> DeserializeHumanReadable(Archive& archive) override { return {}; };

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

    std::vector<ArchiveNodeBase*> SerializeHumanReadable(Archive& archive) override
    {
        return 
        archive.RegisterMembers<SimpleClass*&, std::uint64_t&>(
            { "simple_class_ptr", sc_ptr_ },
            { "u", u_ }
        );
    }

    std::vector<ArchiveNodeBase*> DeserializeHumanReadable(Archive& archive) override { return {}; };

private:
    SimpleClass* sc_ptr_;
    std::uint64_t u_;
};

class B;

class A : public ISerializable
{
public:
    B* b_ptr;

    A(){}

    std::vector<ArchiveNodeBase*> SerializeHumanReadable(Archive& archive) override
    {
        return
            archive.RegisterMembers<B*&>(
                { "b_ptr", b_ptr }
        );
    }

    std::vector<ArchiveNodeBase*> DeserializeHumanReadable(Archive& archive) override { return {}; };
    
};

class B : public ISerializable
{
public:
    A* a_ptr;

    B() {}

    std::vector<ArchiveNodeBase*> SerializeHumanReadable(Archive& archive) override
    {
        return
            archive.RegisterMembers<A*&>(
                { "a_ptr", a_ptr }
        );
    }

    std::vector<ArchiveNodeBase*> DeserializeHumanReadable(Archive& archive) override { return {}; };
};

class C : public ISerializable
{
public:
    C(A a, B b, int i)
    {
        a_ = a;
        b_ = b;
        i_ = i;

        a_.b_ptr = &b_;
        b_.a_ptr = &a_;
    }

    std::vector<ArchiveNodeBase*> SerializeHumanReadable(Archive& archive) override
    {
        return
            archive.RegisterMembers<A&, B&, int&>(
                { "a", a_ },
                { "b", b_ },
                { "i", i_ }
        );
    }

    std::vector<ArchiveNodeBase*> DeserializeHumanReadable(Archive& archive) override { return {}; };

private:
    A a_;
    B b_;
    int i_;
};