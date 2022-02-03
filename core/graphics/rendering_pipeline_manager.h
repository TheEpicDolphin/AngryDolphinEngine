#pragma once

#include <unordered_map>
#include <core/resources/resource_manager.h>

#include "rendering_pipeline.h"

class RenderingPipelineManager
{
public:
	static std::shared_ptr<RenderingPipeline> RenderingPipelineForResourcePath(const char* resource_path);

	static std::shared_ptr<RenderingPipeline> CreateRenderingPipeline(RenderingPipelineInfo info);

private:
	static std::unordered_map<std::string, std::shared_ptr<RenderingPipeline>> loaded_rendering_pipelines_assets_;
	static std::uint32_t next_pipeline_id_;
};
