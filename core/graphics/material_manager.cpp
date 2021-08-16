
#include "material_manager.h"
#include <core/utils/file_helpers.h>
#include <rapidjson/document.h>

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
	pipeline_manager_->LoadPipelineSpecs();
	const std::vector<fs::path> material_spec_files = file_helpers::AllFilePathsInDirectoryWithExtension("//Resources/", ".materialspec");
	for (fs::path material_spec_file : material_spec_files) {
		std::vector<char> file_contents = file_helpers::ReadFileWithPath(material_spec_file);
		rapidjson::Document material_spec_doc;
		material_spec_doc.Parse(file_contents.data());
		material_spec_doc["rendering_pipeline_url"];

		material_spec_doc["uniform_settings"];

		for () {

		}

		const MaterialID material_id = material_id_generator_.CheckoutNewId();
		const std::shared_ptr<RenderingPipeline>& rendering_pipeline = pipeline_manager_->PipelineForPipelineSpecHash(hash);
		const int fileHash = MaterialSpecHashForFilePath(material_spec_file);
		spec_generated_materials_[fileHash] = std::make_shared<Material>(material_id, rendering_pipeline, this);
	}
}

int MaterialManager::MaterialSpecHashForFilePath(std::string)
{
	// TODO: implement hashing of file names
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
