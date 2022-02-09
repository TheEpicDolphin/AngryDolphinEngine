
#include "mesh_manager.h"

/*
#include "rapidxml.hpp"

std::shared_ptr<Mesh> MeshManager::CreateMeshForResourcePath(const char* resource_path_name) {
	std::vector<char> material_asset = ResourceManager::LoadAsset(resource_path_name, "mat");

	rapidxml::xml_document<> material_xml_doc;
	material_xml_doc.parse<0>(material_asset.data());

	const char* rendering_pipeline_path = material_xml_doc.first_node("rendering_pipeline_path")->value();
	std::shared_ptr<RenderingPipeline> rendering_pipeline = RenderingPipelineManager::RenderingPipelineForResourcePath(rendering_pipeline_path);

	std::unordered_map<std::string, UniformValue> uniform_settings;
	rapidxml::xml_node<>* uniform_settings_node = material_xml_doc.first_node("uniform_settings");
	rapidxml::xml_node<>* uniform_setting_node = uniform_settings_node->first_node();
	while (uniform_setting_node != nullptr) {
		std::string uniform_name = uniform_setting_node->first_node("name")->value();
		ShaderDataType shader_data_type = uniform_setting_node->first_node("shader_data_type")->value();
		std::vector<char> uniform_value_data = shader::ValueData(uniform_setting_node->first_node("value")->value());
		uniform_settings[uniform_name] = { 0, shader_data_type , uniform_value_data };
		uniform_setting_node = uniform_setting_node->next_sibling();
	}

	std::shared_ptr<Material> material = std::make_shared<Material>(++next_mesh_id_, { uniform_settings, rendering_pipeline });
}
*/

std::shared_ptr<Mesh> MeshManager::CreateMesh(MeshInfo info) {
	return std::make_shared<Mesh>(++next_mesh_id_, info);
}

std::shared_ptr<Mesh> MeshManager::CreateCubeMeshPrimitive(MeshInfo info, glm::vec3 origin, float side_length) {
	std::shared_ptr<Mesh> mesh = CreateMesh(info);
	std::vector<glm::vec3> cube_verts =
	{
		glm::vec3(0, 0, 0),
		glm::vec3(1, 0, 0),
		glm::vec3(1, 1, 0),
		glm::vec3(0, 1, 0),
		glm::vec3(0, 1, 1),
		glm::vec3(1, 1, 1),
		glm::vec3(1, 0, 1),
		glm::vec3(0, 0, 1)
	};

	const std::vector<std::size_t> indices =
	{
		0, 2, 1, //face front
		0, 3, 2,
		2, 3, 4, //face top
		2, 4, 5,
		1, 2, 5, //face right
		1, 5, 6,
		0, 7, 4, //face left
		0, 4, 3,
		5, 4, 7, //face back
		5, 7, 6,
		0, 6, 7, //face bottom
		0, 1, 6
	};

	for (std::vector<glm::vec3>::iterator it = cube_verts.begin(); it != cube_verts.end(); ++it) {
		*it = (*it - glm::vec3(0.5f, 0.5f, 0.5f) + origin) * side_length;
	}

	mesh->SetVertexPositions(cube_verts);
	mesh->SetTriangleIndices(indices);
	return mesh;
}
