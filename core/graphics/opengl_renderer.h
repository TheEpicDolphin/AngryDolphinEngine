#pragma once

#include <unordered_map>
#include <vector>

#define GLFW_INCLUDE_NONE

#include <GLFW/glfw3.h>
#include <GL/glew.h>

#include "renderer.h"
#include "rendering_pipeline.h"

class OpenGLRenderer : 
	public IRenderer, 
	private MaterialLifecycleEventsListener, 
	private MeshLifecycleEventsListener, 
	private PipelineLifecycleEventsListener
{
public:

	OpenGLRenderer();

	~OpenGLRenderer();

	// IRenderer

	void Initialize(int width, int height);

	void PreloadRenderingPipeline(const std::shared_ptr<RenderingPipeline>& pipeline) override;

	bool RenderFrame(const CameraParams& camera_params, const std::vector<RenderableObject>& renderable_objects) override;

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

	struct PipelineState {
		RenderingPipeline* pipeline;
		GLuint program_id;
	};

	static PipelineState CreatePipelineState(const std::shared_ptr<RenderingPipeline>& pipeline);

	enum MeshDataUsageType {
		MeshDataUsageTypeStatic = 0,
		MeshDataUsageTypeDynamic,
	};

	struct MeshState {
		Mesh* mesh;
		MeshDataUsageType data_usage_type;
		GLuint vao;
		GLuint ibo;
		GLuint* bos;
	};

	static MeshState CreateMeshState(Mesh* mesh);

	struct MaterialState {
		Material* material;
		// TODO: Texture stuff
	};

	static MaterialState CreateMaterialState(Material* mat);

	GLFWwindow* window_;

	std::unordered_map<PipelineID, PipelineState> pipeline_state_map_;
	std::unordered_map<MeshID, MeshState> mesh_state_map_;
	std::unordered_map<MaterialID, MaterialState> material_state_map_;	


	// PipelineLifecycleEventsListener

	void PipelineDidDestroy(PipelineID pipeline_id) override;

	// MeshLifecycleEventsListener

	void MeshVertexAttributeDidChange(Mesh* mesh, std::size_t attribute_index) override;

	void MeshDidDestroy(MeshID mesh_id) override;

	// MaterialLifecycleEventsListener

	void MaterialUniformDidChange(Material* material, std::size_t uniform_index) override;

	//void MaterialTextureDidChange(Material* material, Texture texture) override;

	void MaterialDidDestroy(MaterialID material_id) override;
};
