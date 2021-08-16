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

	bool RenderFrame(std::vector<RenderableObjectInfo> ros);

	void Cleanup();

	std::shared_ptr<RenderingPipeline> CreateRenderingPipeline(RenderingPipelineInfo info);

	std::shared_ptr<Material> CreateMaterial(MaterialInfo info);

	std::shared_ptr<Mesh> CreateMesh(MeshInfo info);

private:
	void LoadRenderingAssets();

	struct IMeshBatch {
		std::shared_ptr<Mesh> mesh;
		GLuint vao;
		GLuint ibo;
		std::vector<glm::mat4> model_matrices;
		virtual void SetupVertexAttributeBuffers();
	};

	struct DynamicMeshBatch : IMeshBatch {
		GLuint vbo;
		DynamicMeshBatch(std::shared_ptr<Mesh> mesh, GLuint vao, GLuint vbo);
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
