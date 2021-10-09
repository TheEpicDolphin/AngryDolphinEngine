
#include <gtest/gtest.h>
#include <vector>
#include <iostream>
#include <algorithm>
#include <fstream>

#include "../archive.h"
#include "serialize_test_objects.h"

TEST(serialize_test_suite, serialize_simple_class_no_pointers_test)
{
    SimpleClass sc(10, 69.69f, "butt");
    Archive archive;

    std::ofstream xmlofile;
    xmlofile.open("serialize_simple_class_no_pointers_test.xml", std::ios::out);
    archive.SerializeHumanReadable(xmlofile, "SimpleClass", sc);
    xmlofile.close();

    SimpleClass sc2;
    std::ifstream xmlifile;
    xmlifile.open("serialize_simple_class_no_pointers_test.xml", std::ios::in);
    archive.DeserializeHumanReadable(xmlifile, "SimpleClass", sc2);
    xmlifile.close();

    ASSERT_EQ(sc, sc2);
}

TEST(serialize_test_suite, serialize_parent_class_no_pointers_test)
{
    SimpleClass sc(10, 69.69f, "butt");
    ParentClass pc({22, 3, -5}, sc);
    Archive archive;
    std::ofstream xmlofile;
    xmlofile.open("serialize_parent_class_no_pointers_test.xml", std::ios::out);
    archive.SerializeHumanReadable(xmlofile, "ParentClass", pc);
    xmlofile.close();

    ParentClass pc2;
    std::ifstream xmlifile;
    xmlifile.open("serialize_parent_class_no_pointers_test.xml", std::ios::in);
    archive.DeserializeHumanReadable(xmlifile, "ParentClass", pc2);
    xmlifile.close();
    
    ASSERT_EQ(pc, pc2);
}

TEST(serialize_test_suite, serialize_parent_class_with_heap_pointer_test)
{
    ParentClassWithHeapPointer pcwhp(120920);
    Archive archive;
    std::ofstream xmlofile;
    xmlofile.open("serialize_parent_class_with_heap_pointer_test.xml", std::ios::out);
    archive.SerializeHumanReadable(xmlofile, "ParentClassWithHeapPointer", pcwhp);
    xmlofile.close();
}

TEST(serialize_test_suite, serialize_cyclical_pointer_test)
{
    C c(A(), B(), 'a');
    Archive archive;
    std::ofstream xmlofile;
    xmlofile.open("serialize_cyclical_pointer_test.xml", std::ios::out);
    archive.SerializeHumanReadable(xmlofile, "c", c);
    xmlofile.close();
    
    C c2(A(), B(), 'd');
    std::ifstream xmlifile;
    xmlifile.open("serialize_cyclical_pointer_test.xml", std::ios::in);
    archive.DeserializeHumanReadable(xmlifile, "c", c2);
    xmlifile.close();

    ASSERT_EQ(c, c2);
}