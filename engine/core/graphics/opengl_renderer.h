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

	void AddRenderableObject(UID id, RenderableObjectInfo info);

	void RemoveRenderableObject(UID id);

	void SetRenderTarget(UID id, RenderTargetInfo info);

	void UnsetRenderTarget(UID id);

	bool RenderFrame();

	void Cleanup();

	std::shared_ptr<RenderingPipeline> CreateRenderingPipeline(RenderingPipelineInfo info);

	std::shared_ptr<Material> CreateMaterial(MaterialInfo info);

private:
	struct MeshBatch {
		std::size_t vertex_count;
		GLuint vao;
		GLuint vbo;
		GLuint ibo;
		std::unordered_map<UID, glm::mat4> model_matrix_map;
	};

	struct MaterialBatch {
		std::vector<MeshID> mesh_ids;
	};

	struct PipelineBatch {
		GLuint program_id;
		std::vector<MaterialID> material_ids;
	};

	typedef struct RenderableID {
		MaterialID material_id;
		MeshID mesh_id;
	} RenderableID;

	GLFWwindow* window_;
	std::unordered_map<PipelineID, PipelineBatch> pipeline_batch_map_;
	std::unordered_map<MaterialID, MaterialBatch> material_batch_map_;
	std::unordered_map<MeshID, MeshBatch> mesh_batch_map_;
	std::unordered_map<UID, RenderableID> renderable_object_map_;

	MeshManager mesh_manager_;
	MaterialManager material_manager_;
	RenderingPipelineManager pipeline_manager_;
};
