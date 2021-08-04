#pragma once

#include <unordered_map>
#include <core/utils/uid_generator.h>

#include "rendering_pipeline.h"

class RenderingPipelineManager : public RenderingPipelineDelegate
{
public:
	// Searches the Resources folder for any .pipelinespec files and creates RenderingPipeline objects from them.
	void LoadPipelineSpecs();

	// Calculates hash for the specified pipeline spec file path. This allows for easier material fetching.
	static int PipelineSpecHashForFilePath(std::string);

	std::shared_ptr<RenderingPipeline> PipelineForPipelineSpecHash(int hash);

	void PipelineDidDestruct(RenderingPipeline* pipeline) override;

	std::shared_ptr<RenderingPipeline> CreatePipeline(RenderingPipelineInfo info);

private:
	std::unordered_map<int, std::shared_ptr<RenderingPipeline>> spec_generated_pipelines_;
	UIDGenerator pipeline_id_generator_;
};
