#pragma once

#include <unordered_map>
#include <core/utils/uid_generator.h>

#include "rendering_pipeline.h"

class RenderingPipelineManager : public RenderingPipelineDelegate
{
public:
	void PipelineDidDestruct(RenderingPipeline* pipeline) override;

	std::shared_ptr<RenderingPipeline> CreatePipeline(RenderingPipelineInfo info);

	std::shared_ptr<RenderingPipeline> CreatePipelineWithHash(RenderingPipelineInfo info, int hash);
	
	std::shared_ptr<RenderingPipeline> LoadPipeline(int hash);

private:
	std::unordered_map<int, std::shared_ptr<RenderingPipeline>> loaded_rendering_pipelines_;
	UIDGenerator pipeline_id_generator_;
};
