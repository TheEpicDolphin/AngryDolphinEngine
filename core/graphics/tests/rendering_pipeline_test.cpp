
#include <gtest/gtest.h>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <algorithm>

#include <core/serialize/archive.h>

#include "../rendering_pipeline.h"

TEST(rendering_pipeline_test_suite, insert_test)
{
    UniformInfo uniform_info;
    uniform_info.name = "";
    uniform_info.data_type = shader::ShaderDataType::Float;
    uniform_info.location = 0;
    uniform_info.array_length = 0;
    uniform_info.category = UniformUsageCategory::MVP;

    Archive archive;

    std::ofstream xmlofile;
    xmlofile.open("serialize_simple_class_no_pointers_test.xml", std::ios::out);
    archive.SerializeHumanReadable(xmlofile, "uniform_info", uniform_info);
    xmlofile.close();
}
