
#include "material_manager.h"
#include <core/utils/file_helpers.h>

static std::unordered_map<std::string, char*> MaterialPropertiesForJson(JSON json) 
{

}

MaterialManager::LoadMaterialSpecs() 
{
	const std::vector<fs::path> material_spec_files = AllFilesInDirectoryWithExtension("//Resources/", "materialspec");
	for (fs::path material_spec_file : material_spec_files) {
		JSON material_spec_json = ParseJSON(material_spec_file);
		const int fileHash = MaterialSpecHashForFilePath(material_spec_file);
		const MaterialID material_id = material_id_generator_.CheckoutNewId();

		const std::string vertex_shader_path = material_spec_json["vertex_shader"];
		const std::string fragment_shader_path = material_spec_json["fragment_shader"];
		const int vertex_shader_path_hash = ShaderManager.ShaderHashForFilePath(vertex_shader_path);
		const int fragment_shader_path_hash = ShaderManager.ShaderHashForFilePath(fragment_shader_path);
		const Shader vertex_shader = shader_manager_->VertexShaderForHash(vertex_shader_path_hash);
		const Shader fragment_shader = shader_manager_->FragmentShaderForHash(fragment_shader_path_hash);
		const std::unordered_map<std::string, char*> material_properties = MaterialPropertiesForJson(material_spec_json);
		spec_generated_materials_[fileHash] = std::make_shared<Material>(material_id, vertex_shader, fragment_shader, material_properties);
	}
}

std::shared_ptr<Material> MaterialForMaterialSpecHash(int hash) 
{
	return spec_generated_materials_[hash];
}
