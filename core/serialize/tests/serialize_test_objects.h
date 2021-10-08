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

    std::vector<ArchiveNodeBase*> RegisterMemberVariablesForSerialization(Archive& archive) override
    {
        return archive.RegisterObjectsForSerialization<int&, float&, std::string&>(
            { "i", i_ },
            { "f", f_ },
            { "s", s_ }
        );
    }

    std::vector<ArchiveNodeBase*> RegisterMemberVariablesForDeserialization(Archive& archive, rapidxml::xml_node<>& xml_node) override
    {
        return archive.RegisterObjectsForDeserialization<int&, float&, std::string&>(
            xml_node,
            i_,
            f_,
            s_
        );
    }

    friend bool operator==(const SimpleClass& lhs, const SimpleClass& rhs) 
    {
        return lhs.i_ == rhs.i_ && lhs.f_ == rhs.f_ && lhs.s_ == rhs.s_;
    }

private:
    int i_;
    float f_;
    std::string s_;
};

class ParentClass : public ISerializable
{
public:
    ParentClass() {

    }

    ParentClass(std::vector<int> vec, SimpleClass sc)
    {
        vec_ = vec;
        sc_ = sc;
    }

    std::vector<ArchiveNodeBase*> RegisterMemberVariablesForSerialization(Archive& archive) override
    {
        return archive.RegisterObjectsForSerialization<std::vector<int>&, SimpleClass&>(
            { "vec", vec_ },
            { "sc", sc_ }
        );
    }

    std::vector<ArchiveNodeBase*> RegisterMemberVariablesForDeserialization(Archive& archive, rapidxml::xml_node<>& xml_node) override
    {
        return archive.RegisterObjectsForDeserialization<std::vector<int>&, SimpleClass&>(
            xml_node,
            vec_,
            sc_
        );
    }

    friend bool operator==(const ParentClass& lhs, const ParentClass& rhs)
    {
        return (lhs.vec_.size() == rhs.vec_.size() 
            && std::equal(lhs.vec_.begin(), lhs.vec_.end(), rhs.vec_.begin())) 
            && lhs.sc_ == rhs.sc_;
        
    }

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

    std::vector<ArchiveNodeBase*> RegisterMemberVariablesForSerialization(Archive& archive) override
    {
        return archive.RegisterObjectsForSerialization<SimpleClass*&, std::uint64_t&>(
            { "simple_class_ptr", sc_ptr_ },
            { "u", u_ }
        );
    }

    std::vector<ArchiveNodeBase*> RegisterMemberVariablesForDeserialization(Archive& archive, rapidxml::xml_node<>& xml_node) override
    {
        return archive.RegisterObjectsForDeserialization<SimpleClass*&, std::uint64_t&>(
            xml_node,
            sc_ptr_,
            u_
        );
    }

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

    std::vector<ArchiveNodeBase*> RegisterMemberVariablesForSerialization(Archive& archive) override
    {
        return archive.RegisterObjectsForSerialization<B*&>(
            { "b_ptr", b_ptr }
        );
    }

    std::vector<ArchiveNodeBase*> RegisterMemberVariablesForDeserialization(Archive& archive, rapidxml::xml_node<>& xml_node) override
    {
        return archive.RegisterObjectsForDeserialization<B*&>(
            xml_node,
            b_ptr
        );
    }
};

class B : public ISerializable
{
public:
    A* a_ptr;

    B() {}

    std::vector<ArchiveNodeBase*> RegisterMemberVariablesForSerialization(Archive& archive) override
    {
        return archive.RegisterObjectsForSerialization<A*&>(
            { "a_ptr", a_ptr }
        );
    }

    std::vector<ArchiveNodeBase*> RegisterMemberVariablesForDeserialization(Archive& archive, rapidxml::xml_node<>& xml_node) override
    {
        return archive.RegisterObjectsForDeserialization<A*&>(
            xml_node,
            a_ptr
        );
    }
};

class C : public ISerializable
{
public:
    C() {}

    C(A a, B b, int i)
    {
        a_ = a;
        b_ = b;
        i_ = i;

        a_.b_ptr = &b_;
        b_.a_ptr = &a_;

        sc_sp_ = std::make_shared<SimpleClass>(-1, 420.420f, "squid game");
    }

    std::vector<ArchiveNodeBase*> RegisterMemberVariablesForSerialization(Archive& archive) override
    {
        return archive.RegisterObjectsForSerialization<A&, B&, int&, std::shared_ptr<SimpleClass>&>(
            { "a", a_ },
            { "b", b_ },
            { "i", i_ },
            { "sc_sp", sc_sp_ }
        );
    }

    std::vector<ArchiveNodeBase*> RegisterMemberVariablesForDeserialization(Archive& archive, rapidxml::xml_node<>& xml_node) override
    {
        return archive.RegisterObjectsForDeserialization<A&, B&, int&, std::shared_ptr<SimpleClass>&>(
            xml_node,
            a_,
            b_,
            i_,
            sc_sp_
        );
    }
    
    /*
    friend bool operator==(const C& lhs, const C& rhs)
    {
        return (lhs.a_== rhs.vec_.size()
            && std::equal(lhs.vec_.begin(), lhs.vec_.end(), rhs.vec_.begin()))
            && lhs.sc_ == rhs.sc_;
    }
    */

private:
    A a_;
    B b_;
    int i_;
    std::shared_ptr<SimpleClass> sc_sp_;
};