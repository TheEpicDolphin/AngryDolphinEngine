
#include <gtest/gtest.h>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <algorithm>

#include <core/serialize/archive.h>

#include "../rendering_pipeline.h"

TEST(rendering_pipeline_test_suite, rendering_pipeline_for_resource_path_test)
{
    UniformInfo uniform_info;
    uniform_info.name = "UNIFORM NAME";
    uniform_info.data_type = shader::ShaderDataType::Float;
    uniform_info.location = 0;
    uniform_info.array_length = 0;
    uniform_info.category = UniformUsageCategory::MVP;

    Archive archive;

    std::ofstream xmlofile;
    xmlofile.open("serialize_uniform_test.xml", std::ios::out);
    archive.SerializeHumanReadable(xmlofile, "uniform_info", uniform_info);
    xmlofile.close();
}
