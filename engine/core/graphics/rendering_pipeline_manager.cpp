
#include "rendering_pipeline_manager.h"
#include <core/utils/file_helpers.h>
#include <rapidjson/document.h>

void RenderingPipelineManager::LoadPipelineSpecs()
{
	const std::vector<fs::path> pipeline_spec_files = file_helpers::AllFilePathsInDirectoryWithExtension("//Resources/", ".pipelinespec");
	for (fs::path pipeline_spec_file : pipeline_spec_files) {
		std::vector<char> file_contents = file_helpers::ReadFileWithPath(pipeline_spec_file);
		rapidjson::Document pipeline_spec_doc;
		pipeline_spec_doc.Parse(file_contents.data());
		const int fileHash = PipelineSpecHashForFilePath(material_spec_file);

		pipeline_spec_doc["stages"];

		std::unordered_map<std::string, Uniform> uniforms;
		std::vector<Shader> stages;
		for () {
			std::vector<char> code = ["code"];
			spec_generated_materials_[fileHash] = Shader(code);
		}

		const PipelineID pipeline_id = pipeline_id_generator_.CheckoutNewId();
		spec_generated_pipelines_[fileHash] = std::make_shared<RenderingPipeline>(pipeline_id, stages, uniforms, this);
	}
}

int RenderingPipelineManager::PipelineSpecHashForFilePath(std::string)
{
	// TODO: implement hashing of file names
	return 0;
}

std::shared_ptr<RenderingPipeline> RenderingPipelineManager::PipelineForPipelineSpecHash(int hash)
{
	return spec_generated_pipelines_[hash];
}

void RenderingPipelineManager::PipelineDidDestruct(RenderingPipeline* pipeline)
{
	pipeline_id_generator_.ReturnId(pipeline->GetInstanceID());
}
