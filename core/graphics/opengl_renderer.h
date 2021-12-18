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

	void RegisterRenderingPipeline(std::shared_ptr<RenderingPipeline> rendering_pipeline) override;

	void RegisterMaterial(std::shared_ptr<Material> material) override;

	void RegisterMesh(std::shared_ptr<Mesh> mesh) override;

private:
	void LoadRenderingAssets();

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
		std::vector<RenderableObjectInstance> instances;
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
