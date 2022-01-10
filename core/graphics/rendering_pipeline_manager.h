#pragma once

#include <unordered_map>
#include <core/utils/uid_generator.h>
#include <core/resources/resource_manager.h>

#include "rendering_pipeline.h"

class RenderingPipelineManager : private RenderingPipelineDelegate
{
public:
	static std::shared_ptr<RenderingPipeline> RenderingPipelineForResourcePath(const char* resource_path);

	static std::shared_ptr<RenderingPipeline> CreateRenderingPipeline(RenderingPipelineInfo info);

private:
	void PipelineDidDestruct(RenderingPipeline* pipeline) override;

	static std::unordered_map<std::string, std::shared_ptr<RenderingPipeline>> loaded_rendering_pipelines_assets_;
	static std::unique_ptr<UIDGenerator> pipeline_id_generator_;
};
