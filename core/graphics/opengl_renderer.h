#pragma once

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <unordered_map>
#include <vector>

#include "renderer.h"
#include "rendering_pipeline.h"

class OpenGLRenderer : 
	IRenderer, 
	MaterialLifecycleEventsListener, 
	MeshLifecycleEventsListener, 
	PipelineLifecycleEventsListener
{
public:
	void Initialize(int width, int height);

	void PreloadRenderingPipeline(const std::shared_ptr<RenderingPipeline>& pipeline) override;

	bool RenderFrame(const std::vector<CameraParams>& cameras, const std::vector<RenderableObject>& renderable_objects) override;

	void Cleanup() override;

private:

	struct RenderableObjectBatchKey {
		PipelineID pipeline_id;
		MeshID mesh_id;
		MaterialID material_id;

		bool operator <(const RenderableObjectBatchKey& rhs) const
		{
			if (pipeline_id == rhs.pipeline_id) {
				if (mesh_id == rhs.mesh_id) {
					return material_id < rhs.material_id;
				}
				return mesh_id < rhs.mesh_id;
			}
			return pipeline_id < rhs.pipeline_id;
		}
	};

	struct RenderableObjectInstance {
		glm::mat4 model_transform;
		std::vector<glm::mat4> bone_transforms;
	};

	struct RenderableObjectBatch {
		Mesh* mesh;
		Material* material;
		std::vector<RenderableObjectInstance> instances;
	};

	enum MeshDataUsageType {
		MeshDataUsageTypeStatic = 0,
		MeshDataUsageTypeDynamic,
	};

	struct MeshConfiguration {
		MeshDataUsageType data_usage_type;
		GLuint vao;
		GLuint ibo;
		GLuint vbo;
		void SetupVertexAttributeBuffers(Mesh *mesh);
	};

	static MeshConfiguration CreateMeshConfiguration(Mesh* mesh);

	struct PipelineConfiguration {
		GLuint program_id;
	};

	static PipelineConfiguration CreatePipelineConfiguration(const std::shared_ptr<RenderingPipeline>& pipeline);

	GLFWwindow* window_;

	std::unordered_map<MeshID, MeshConfiguration> mesh_config_map_;
	//std::unordered_map<MaterialID, MaterialConfiguration> material_config_map_;
	std::unordered_map<PipelineID, PipelineConfiguration> pipeline_config_map_;	
};
