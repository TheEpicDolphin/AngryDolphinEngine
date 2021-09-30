
#include <gtest/gtest.h>
#include <string>
#include <vector>
#include <iostream>
#include <algorithm>
#include <fstream>

#include "../archive.h"
#include "../serializable.h"

class SimpleClass : public ISerializable
{
public:
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

TEST(serialize_test_suite, serialize_simple_class_no_pointers_test)
{
    SimpleClass sc(10, 69.69f, "butt");
    Archive archive;
    std::filebuf xmlofilebuf;
    xmlofilebuf.open("serialize_simple_class_no_pointers_test.txt", std::ios::out);
    std::ostream xmlostream(&xmlofilebuf);
    archive.SerializedHumanReadable(xmlostream, "SimpleClass", sc);
    xmlofilebuf.close();
    //ASSERT_EQ(*node_value, node_1);
}
