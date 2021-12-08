
#include "rendering_pipeline_manager.h"
#include <core/utils/file_helpers.h>

void RenderingPipelineManager::LoadRenderingPipelines()
{
	// Recursively searches for .rp files in the project's resources folder.
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

void RenderingPipelineManager::PipelineDidDestruct(RenderingPipeline* pipeline)
{
	pipeline_id_generator_.ReturnId(pipeline->GetInstanceID());
}

std::shared_ptr<RenderingPipeline> RenderingPipelineManager::CreatePipeline(RenderingPipelineInfo info)
{
	std::vector<Shader> stages;

	for (std::unordered_map stages : info["stages"]) {
		stages.push_back(Shader(stages["type"], stages["code"]));
	}

	const PipelineID pipeline_id = pipeline_id_generator_.CheckoutNewId();
	return std::make_shared<RenderingPipeline>(pipeline_id, stages, uniforms, this);
}

std::shared_ptr<RenderingPipeline> RenderingPipelineManager::CreatePipelineWithHash(RenderingPipelineInfo info, int hash)
{
	loaded_rendering_pipelines_[hash] = CreatePipeline(info);
}

std::shared_ptr<RenderingPipeline> RenderingPipelineManager::LoadPipeline(int hash) 
{
	return loaded_rendering_pipelines_[hash];
}
