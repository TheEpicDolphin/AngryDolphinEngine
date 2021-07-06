
#include "material_manager.h"
#include <core/utils/file_helpers.h>
#include <rapidjson/document.h>

enum MaterialPropertyType {
	MaterialPropertyTypeBool,
	MaterialPropertyTypeInt,
	MaterialPropertyTypeUInt,
	MaterialPropertyTypeFloat,
	MaterialPropertyTypeDouble
};

static void SetMaterialPropertiesForMaterial(Material *material, )
{
	for () {
		switch (property_type)
		{
		default:
			break;
		}
	}
}

void MaterialManager::LoadMaterialSpecs() 
{
	const std::vector<fs::path> material_spec_files = file_helpers::AllFilePathsInDirectoryWithExtension("//Resources/", ".materialspec");
	for (fs::path material_spec_file : material_spec_files) {
		std::vector<char> file_contents = file_helpers::ReadFileWithPath(material_spec_file);
		rapidjson::Document material_spec_doc;
		material_spec_doc.Parse(file_contents.data());
		material_spec_doc["properties"];
		JSON material_spec_json = ParseJSON(material_spec_file);
		const int fileHash = MaterialSpecHashForFilePath(material_spec_file);

		const std::string vertex_shader_path = material_spec_doc["vertex_shader"];
		const std::string fragment_shader_path = material_spec_doc["fragment_shader"];
		const int vertex_shader_path_hash = ShaderManager.ShaderHashForFilePath(vertex_shader_path);
		const int fragment_shader_path_hash = ShaderManager.ShaderHashForFilePath(fragment_shader_path);

		const MaterialID material_id = material_id_generator_.CheckoutNewId();
		const std::shared_ptr<Shader> vertex_shader = shader_manager_->VertexShaderForHash(vertex_shader_path_hash);
		const std::shared_ptr<Shader> fragment_shader = shader_manager_->FragmentShaderForHash(fragment_shader_path_hash);
		const std::unordered_map<std::string, char*> material_properties = MaterialPropertiesForJson(material_spec_json);
		spec_generated_materials_[fileHash] = std::make_shared<Material>(material_id, vertex_shader, fragment_shader, material_properties, this);
	}
}

int MaterialManager::MaterialSpecHashForFilePath(std::string)
{
	// TODO: implement hashing
	return 0;
}

std::shared_ptr<Material> MaterialManager::MaterialForMaterialSpecHash(int hash)
{
	return spec_generated_materials_[hash];
}

void MaterialManager::MaterialDidDestruct(Material *material)
{
	material_id_generator_.ReturnId(material->GetInstanceID());
}
