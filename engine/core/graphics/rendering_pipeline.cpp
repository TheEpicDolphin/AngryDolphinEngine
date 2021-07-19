
#include "rendering_pipeline.h"

RenderingPipeline::RenderingPipeline(PipelineID pipeline_id, std::vector<Shader> shader_stages)
{
	id_ = pipeline_id;
	shader_stages_ = shader_stages;
}

const PipelineID& GetInstanceID() 
{
	return id_;
}
