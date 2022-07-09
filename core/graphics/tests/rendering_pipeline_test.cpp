
#include <gtest/gtest.h>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <algorithm>

#include <core/serialize/archive.h>
#include <core/graphics/rendering_pipeline.h>

std::string NameForStageType(shader::ShaderStageType type)
{
    switch (type)
    {
    case shader::ShaderStageType::Vertex:
        return "Vertex Shader Stage";
    case shader::ShaderStageType::Geometry:
        return "Geometry Shader Stage";
    case shader::ShaderStageType::Fragment:
        return "Fragment Shader Stage";
    case shader::ShaderStageType::Compute:
        return "Compute Shader Stage";
    }
    return "";
}

TEST(rendering_pipeline_test_suite, rendering_pipeline_serialization_test)
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
    vert_shader.code = "vertex shader code...";

    shader::Shader frag_shader;
    frag_shader.type = shader::ShaderStageType::Fragment;
    frag_shader.code = "fragment shader code...";

    RenderingPipelineInfo rp_info;
    rp_info.mvp_uniform = mvp_uniform_info;
    rp_info.material_uniforms = { main_color_uniform_info };
    rp_info.vertex_attributes = { vertex_pos_info };
    rp_info.shader_stages = { vert_shader, frag_shader };

    Archive archive;
    rapidxml::xml_document<>* xml_doc = new rapidxml::xml_document<>();
    archive.SerializeHumanReadable(*xml_doc, "uniform_info", rp_info);

    std::ofstream xmlofile;
    xmlofile.open("serialized_rendering_pipeline_info.xml", std::ios::out);
    xmlofile << *xml_doc;
    
    xml_doc->clear();
    xmlofile.close();
    delete xml_doc;
}

TEST(rendering_pipeline_test_suite, rendering_pipeline_for_resource_path_test)
{
    std::shared_ptr<RenderingPipeline> rp = RenderingPipeline::RenderingPipelineForResourcePath("standard_rendering_pipeline/standard.xml");
    const std::vector<shader::Shader>& shader_stages = rp->ShaderStages();
    for (auto it = shader_stages.begin(); it != shader_stages.end(); ++it) {
        std::cout << NameForStageType(it->type) << std::endl;
        std::cout << it->code << std::endl;
    }

    std::shared_ptr<RenderingPipeline> rp_reloaded = RenderingPipeline::RenderingPipelineForResourcePath("standard_rendering_pipeline/standard.xml");
    ASSERT_EQ(rp, rp_reloaded);
}
