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

	void SetRenderTarget(UID id, RenderTargetInfo info) override;

	void UnsetRenderTarget(UID id) override;

	bool RenderFrame(const std::vector<RenderableObject>& renderable_objects) override;

	void Cleanup() override;

	std::shared_ptr<RenderingPipeline> CreateRenderingPipeline(RenderingPipelineInfo info) override;

	std::unique_ptr<Material> CreateUniqueMaterial(MaterialInfo info) override;

	std::shared_ptr<Material> CreateSharedMaterial(MaterialInfo info) override;

	std::unique_ptr<Mesh> CreateUniqueMesh(MeshInfo info) override;

	std::shared_ptr<Mesh> CreateSharedMesh(MeshInfo info) override;

private:
	void LoadRenderingAssets();

	struct RenderableObjectKey {
		PipelineID pipeline_id;
		MeshID mesh_id;
		MaterialID material_id;
	};

	struct RenderableObjectInstance {
		Mesh* mesh;
		glm::mat4 model_transform;
		std::vector<glm::mat4> bone_transforms;
	};

	enum MeshDataUsageType {
		MeshDataUsageTypeStream = 0,
		MeshDataUsageTypeStatic,
		MeshDataUsageTypeDynamic,
	};

	struct MeshConfiguration {
		MeshDataUsageType data_usage_type;
		GLuint vao;
		GLuint ibo;
		GLuint vbo;
		void SetupVertexAttributeBuffers(Mesh *mesh);
	};

	struct PipelineConfiguration {
		GLuint program_id;
	};

	GLFWwindow* window_;

	std::unordered_map<MeshID, MeshConfiguration> mesh_config_map_;
	//std::unordered_map<MaterialID, MaterialConfiguration> material_config_map_;
	std::unordered_map<PipelineID, PipelineConfiguration> pipeline_config_map_;

	MeshManager mesh_manager_;
	MaterialManager material_manager_;
	RenderingPipelineManager pipeline_manager_;

	std::unordered_map<std::string, int> asset_filepath_hasher_;
};
