#pragma once

#include <unordered_map>
#include <core/utils/uid_generator.h>

#include "rendering_pipeline.h"

// Make this a static class
class ResourceManager
{
public:
	void PipelineDidDestruct(RenderingPipeline* pipeline) override;

	std::shared_ptr<RenderingPipeline> CreateRenderingPipeline(RenderingPipelineInfo info);

	std::shared_ptr<RenderingPipeline> CreateRenderingPipelineWithHash(RenderingPipelineInfo info, int hash);

	std::shared_ptr<RenderingPipeline> LoadPipeline(int hash);

	static 

private:
	static std::unordered_map<std::string, int> resource_name_to_hash_map_;
	std::unordered_map<int, std::shared_ptr<RenderingPipeline>> loaded_rendering_pipelines_;
};