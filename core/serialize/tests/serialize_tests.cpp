
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
    rapidxml::xml_document<> xml_doc;
    archive.SerializeHumanReadable(xml_doc, "SimpleClass", sc);

    std::ofstream xmlofile;
    xmlofile.open("serialize_simple_class_no_pointers_test.xml", std::ios::out);
    xmlofile << xml_doc;
    xmlofile.close();
    xml_doc.clear();


    SimpleClass sc2;
    std::ifstream xmlifile;
    xmlifile.open("serialize_simple_class_no_pointers_test.xml", std::ios::in);
    std::vector<char> buffer((std::istreambuf_iterator<char>(xmlifile)), std::istreambuf_iterator<char>());
    xmlifile.close();

    buffer.push_back('\0');
    xml_doc.parse<0>(buffer.data());
    archive.DeserializeHumanReadable(xml_doc, sc2);
    xml_doc.clear();

    ASSERT_EQ(sc, sc2);
}

TEST(serialize_test_suite, serialize_parent_class_no_pointers_test)
{
    SimpleClass sc(10, 69.69f, "butt");
    ParentClass pc({22, 3, -5}, sc);
    Archive archive;
    rapidxml::xml_document<> xml_doc;
    archive.SerializeHumanReadable(xml_doc, "ParentClass", pc);

    std::ofstream xmlofile;
    xmlofile.open("serialize_parent_class_no_pointers_test.xml", std::ios::out);
    xmlofile << xml_doc;
    xmlofile.close();

    ParentClass pc2;
    std::ifstream xmlifile;
    xmlifile.open("serialize_parent_class_no_pointers_test.xml", std::ios::in);
    std::vector<char> buffer((std::istreambuf_iterator<char>(xmlifile)), std::istreambuf_iterator<char>());
    xmlifile.close();
    
    buffer.push_back('\0');
    xml_doc.parse<0>(buffer.data());
    archive.DeserializeHumanReadable(xml_doc, pc2);
    xml_doc.clear();

    ASSERT_EQ(pc, pc2);
}

TEST(serialize_test_suite, serialize_parent_class_with_heap_pointer_test)
{
    ParentClassWithHeapPointer pcwhp(120920);
    Archive archive;
    rapidxml::xml_document<> xml_doc;
    archive.SerializeHumanReadable(xml_doc, "ParentClassWithHeapPointer", pcwhp);

    std::ofstream xmlofile;
    xmlofile.open("serialize_parent_class_with_heap_pointer_test.xml", std::ios::out);
    xmlofile << xml_doc;
    xmlofile.close();
    xml_doc.clear();
}

TEST(serialize_test_suite, serialize_cyclical_pointer_test)
{
    C c(A(), B(), 'a');
    Archive archive;
    rapidxml::xml_document<> xml_doc;
    archive.SerializeHumanReadable(xml_doc, "c", c);

    std::ofstream xmlofile;
    xmlofile.open("serialize_cyclical_pointer_test.xml", std::ios::out);
    xmlofile << xml_doc;
    xmlofile.close();
    xml_doc.clear();
    
    C c2(A(), B(), 'd');
    std::ifstream xmlifile;
    xmlifile.open("serialize_cyclical_pointer_test.xml", std::ios::in);
    std::vector<char> buffer((std::istreambuf_iterator<char>(xmlifile)), std::istreambuf_iterator<char>());
    xmlifile.close();

    buffer.push_back('\0');
    xml_doc.parse<0>(buffer.data());
    archive.DeserializeHumanReadable(xml_doc, c2);
    xml_doc.clear();

    ASSERT_EQ(c, c2);
}