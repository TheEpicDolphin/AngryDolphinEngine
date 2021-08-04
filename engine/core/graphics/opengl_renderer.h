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
		std::shared_ptr<Mesh> mesh;
		GLuint vbo;
		GLuint ibo;
		std::unordered_map<UID, glm::mat4> model_matrix_map;
	};

	struct MaterialBatch {
		std::shared_ptr<Material> material;
		GLuint vao;
		std::unordered_map<MeshID, MeshBatch> mesh_batch_map;
	};

	struct PipelineBatch {
		std::shared_ptr<RenderingPipeline> pipeline;
		GLuint program_id;
		std::unordered_map<MaterialID, MaterialBatch> material_batch_map;
	};

	typedef struct RenderableID {
		MaterialID material_id;
		MeshID mesh_id;
	} RenderableID;

	GLFWwindow* window_;
	std::unordered_map<PipelineID, PipelineBatch> pipeline_batch_map_;
	std::unordered_map<UID, RenderableID> renderable_object_map_;

	MaterialManager material_manager_;
	RenderingPipelineManager pipeline_manager_;

	static void DestroyWindow();

	static MeshBatch CreateMeshBatch(GLuint& vao, Material& material, Mesh& mesh);
};
