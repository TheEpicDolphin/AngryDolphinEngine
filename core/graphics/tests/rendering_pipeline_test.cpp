
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
    UniformInfo mvp_uniform_info;
    mvp_uniform_info.name = "mvp_uniform";
    mvp_uniform_info.data_type = shader::ShaderDataType::Matrix4f;
    mvp_uniform_info.location = 0;
    mvp_uniform_info.array_length = 1;
    mvp_uniform_info.category = UniformUsageCategory::MVP;

    UniformInfo main_color_uniform_info;
    main_color_uniform_info.name = "main_color";
    main_color_uniform_info.data_type = shader::ShaderDataType::Vector4f;
    main_color_uniform_info.location = 1;
    main_color_uniform_info.array_length = 1;
    main_color_uniform_info.category = UniformUsageCategory::MVP;

    VertexAttributeInfo vertex_pos_info;
    vertex_pos_info.name = "position";
    vertex_pos_info.data_type = shader::ShaderDataType::Vector3f;
    vertex_pos_info.location = 2;
    vertex_pos_info.dimension = 3;
    vertex_pos_info.format = 4;
    vertex_pos_info.category = VertexAttributeUsageCategory::Position;

    shader::Shader vert_shader;
    vert_shader.type = shader::ShaderStageType::Vertex;
    vert_shader.code = "I AM VERT SHADER CODE";

    shader::Shader frag_shader;
    frag_shader.type = shader::ShaderStageType::Fragment;
    frag_shader.code = "I AM FRAG SHADER CODE";


    RenderingPipelineInfo rp_info;
    rp_info.mvp_uniform = mvp_uniform_info;
    rp_info.material_uniforms = { main_color_uniform_info };
    rp_info.vertex_attributes = { vertex_pos_info };
    rp_info.shader_stages = { vert_shader, frag_shader };

    Archive archive;

    std::ofstream xmlofile;
    xmlofile.open("serialized_rendering_pipeline_info.xml", std::ios::out);
    archive.SerializeHumanReadable(xmlofile, "uniform_info", rp_info);
    xmlofile.close();
}
