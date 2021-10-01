
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
    std::filebuf xmlofilebuf;
    xmlofilebuf.open("serialize_simple_class_no_pointers_test.txt", std::ios::out);
    std::ostream xmlostream(&xmlofilebuf);
    archive.SerializeHumanReadable(xmlostream, "SimpleClass", sc);
    xmlofilebuf.close();
    //ASSERT_EQ(*node_value, node_1);
}

TEST(serialize_test_suite, serialize_parent_class_no_pointers_test)
{
    SimpleClass sc(10, 69.69f, "butt");
    ParentClass pc({22, 3, -5}, sc);
    Archive archive;
    std::filebuf xmlofilebuf;
    xmlofilebuf.open("serialize_parent_class_no_pointers_test.txt", std::ios::out);
    std::ostream xmlostream(&xmlofilebuf);
    archive.SerializeHumanReadable(xmlostream, "ParentClass", pc);
    xmlofilebuf.close();
    //ASSERT_EQ(*node_value, node_1);
}

TEST(serialize_test_suite, serialize_parent_class_with_heap_pointer_test)
{
    ParentClassWithHeapPointer pcwhp(120920);
    Archive archive;
    std::filebuf xmlofilebuf;
    xmlofilebuf.open("serialize_parent_class_no_pointers_test.txt", std::ios::out);
    std::ostream xmlostream(&xmlofilebuf);
    archive.SerializeHumanReadable(xmlostream, "ParentClassWithHeapPointer", pcwhp);
    xmlofilebuf.close();
    //ASSERT_EQ(*node_value, node_1);
}
