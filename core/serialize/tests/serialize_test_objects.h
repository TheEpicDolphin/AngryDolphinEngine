#pragma once

#include <vector>
#include <string>

#include "../serializable.h"

class SimpleClass
{
public:
    SimpleClass() {};

    SimpleClass(int i, float f, std::string s)
    {
        i_ = i;
        f_ = f;
        s_ = s;
    }

    friend bool operator==(const SimpleClass& lhs, const SimpleClass& rhs) 
    {
        std::cout << lhs.i_ << " " << lhs.f_ << " " << lhs.s_ << std::endl;
        std::cout << rhs.i_ << " " << rhs.f_ << " " << rhs.s_ << std::endl;
        return lhs.i_ == rhs.i_ && lhs.f_ == rhs.f_ && lhs.s_ == rhs.s_;
    }

    SERIALIZE_MEMBERS(i_, f_, s_)

private:
    int i_;
    float f_;
    std::string s_;
};

class ParentClass
{
public:
    ParentClass() {

    }

    ParentClass(std::vector<int> vec, SimpleClass sc)
    {
        vec_ = vec;
        sc_ = sc;
    }

    friend bool operator==(const ParentClass& lhs, const ParentClass& rhs)
    {
        return (lhs.vec_.size() == rhs.vec_.size() 
            && std::equal(lhs.vec_.begin(), lhs.vec_.end(), rhs.vec_.begin())) 
            && lhs.sc_ == rhs.sc_;
        
    }

    SERIALIZE_MEMBERS(vec_, sc_)

private:
    std::vector<int> vec_;
    SimpleClass sc_;
};

class ParentClassWithHeapPointer
{
public:
    ParentClassWithHeapPointer(std::uint64_t u)
    {
        sc_ptr_ = new SimpleClass(50, -3.14f, "I was allocated on the heap!");
        u_ = u;
    }

    SERIALIZE_MEMBERS(sc_ptr_, u_)

private:
    SimpleClass* sc_ptr_;
    std::uint64_t u_;
};

class B;

class A
{
public:
    B* b_ptr;

    A(){}

    SERIALIZE_MEMBERS(b_ptr)
};

class B
{
public:
    A* a_ptr;

    B() {}

    SERIALIZE_MEMBERS(a_ptr)
};

class C
{
public:

    C(A a, B b, char c)
    {
        a_ = a;
        b_ = b;
        c_ = c;

        i_arr_[0] = 98;
        i_arr_[1] = 99;
        i_arr_[2] = 100;

        i_2d_arr_[0][0] = 12;
        i_2d_arr_[0][1] = 13;
        i_2d_arr_[1][0] = 14;
        i_2d_arr_[1][1] = 15;

        a_.b_ptr = &b_;
        b_.a_ptr = &a_;

        sc_sp_ = std::make_shared<SimpleClass>(-1, 420.420f, "squid game");
    }
    
    friend bool operator==(const C& lhs, const C& rhs)
    {
        return (lhs.a_.b_ptr == &lhs.b_ && rhs.a_.b_ptr == &rhs.b_)
            && (lhs.b_.a_ptr == &lhs.a_ && rhs.b_.a_ptr == &rhs.a_)
            && lhs.c_ == rhs.c_
            && (lhs.i_arr_[0] == rhs.i_arr_[0] && lhs.i_arr_[1] == rhs.i_arr_[1] && lhs.i_arr_[2] == rhs.i_arr_[2])
            && (lhs.i_2d_arr_[0][0] == rhs.i_2d_arr_[0][0] && lhs.i_2d_arr_[0][1] == rhs.i_2d_arr_[0][1] && lhs.i_2d_arr_[1][0] == rhs.i_2d_arr_[1][0] && lhs.i_2d_arr_[1][1] == rhs.i_2d_arr_[1][1])
            && *lhs.sc_sp_.get() == *rhs.sc_sp_.get();
    }

    SERIALIZE_MEMBERS(a_, b_, c_, i_arr_, i_2d_arr_, sc_sp_)

private:
    A a_;
    B b_;
    char c_;
    int i_arr_[3];
    int i_2d_arr_[2][2];
    std::shared_ptr<SimpleClass> sc_sp_;
};