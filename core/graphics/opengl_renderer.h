#pragma once

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <unordered_map>
#include <vector>

#include "renderer.h"
#include "rendering_pipeline.h"
#include "material_manager.h"
#include "rendering_pipeline_manager.h"

class OpenGLRenderer : IRenderer
{
public:
	void Initialize(int width, int height);

	void SetRenderTarget(UID id, RenderTargetInfo info);

	void UnsetRenderTarget(UID id);

	bool RenderFrame(const std::vector<RenderableObject>& renderable_objects);

	void Cleanup();

	std::shared_ptr<RenderingPipeline> CreateRenderingPipeline(RenderingPipelineInfo info);

	std::shared_ptr<Material> CreateMaterial(MaterialInfo info);

	std::shared_ptr<Mesh> CreateMesh(MeshInfo info);

private:
	void LoadRenderingAssets();

	struct RenderableObjectInstance {
		glm::mat4 model_transform;
		std::vector<glm::mat4> bone_transforms;
	};

	struct IMeshBatch {
		Mesh* mesh;
		GLuint vao;
		GLuint ibo;
		std::vector<RenderableObjectInstance> renderable_object_instances;
		virtual void SetupVertexAttributeBuffers() = 0;
	};

	struct DynamicMeshBatch : IMeshBatch {
		GLuint vbo;
		DynamicMeshBatch(Mesh* mesh, GLuint vao, GLuint vbo);
		void SetupVertexAttributeBuffers();
	};

	struct PipelineBatch {
		GLuint program_id;
		std::vector<MeshID> mesh_ids;
	};

	GLFWwindow* window_;
	std::unordered_map<MeshID, IMeshBatch*> mesh_batch_map_;
	std::unordered_map<PipelineID, PipelineBatch> pipeline_batch_map_;

	MeshManager mesh_manager_;
	MaterialManager material_manager_;
	RenderingPipelineManager pipeline_manager_;

	std::unordered_map<std::string, int> asset_filepath_hasher_;
};
