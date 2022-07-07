#pragma once

#include <unordered_map>
#include <vector>

#include <GL/glew.h>
#include <core/definitions/graphics/renderer.h>

#include "material.h"
#include "mesh.h"
#include "rendering_pipeline.h"

typedef RenderingPipeline* PipelineHandle;
typedef Mesh* MeshHandle;
typedef Material* MaterialHandle;

class OpenGLRenderer : 
	public IRenderer,
	private MaterialLifecycleEventsListener,
	private MeshLifecycleEventsListener,
	private PipelineLifecycleEventsListener
{
public:
	// I am sitting here because it makes Jiaming suddenly hungry when i work on my game engine. I'm not actually coding.
	OpenGLRenderer();

	~OpenGLRenderer();

	void PreloadRenderingPipeline(const std::shared_ptr<RenderingPipeline>& pipeline) override;

	void RenderFrame(const CameraParams& camera_params, const std::vector<RenderableObject>& renderable_objects) override;

	void Cleanup() override;

private:

	struct RenderableObjectBatchKey {
		PipelineHandle pipeline_handle;
		MeshHandle mesh_handle;
		MaterialHandle material_handle;

		bool operator <(const RenderableObjectBatchKey& rhs) const
		{
			if (pipeline_handle == rhs.pipeline_handle) {
				if (mesh_handle == rhs.mesh_handle) {
					return material_handle < rhs.material_handle;
				}
				return mesh_handle < rhs.mesh_handle;
			}
			return pipeline_handle < rhs.pipeline_handle;
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

	enum class MeshDataUsageType {
		Static = 0,
		Dynamic,
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

	std::unordered_map<PipelineHandle, PipelineState> pipeline_state_map_;
	std::unordered_map<MeshHandle, MeshState> mesh_state_map_;
	std::unordered_map<MaterialHandle, MaterialState> material_state_map_;	


	// PipelineLifecycleEventsListener

	void PipelineDidDestroy(RenderingPipeline* pipeline) override;

	// MeshLifecycleEventsListener

	void MeshVertexAttributeDidChange(Mesh* mesh, std::size_t attribute_index) override;

	void MeshDidDestroy(Mesh* mesh) override;

	// MaterialLifecycleEventsListener

	void MaterialUniformDidChange(Material* material, std::size_t uniform_index) override;

	//void MaterialTextureDidChange(Material* material, Texture texture) override;

	void MaterialDidDestroy(Material* material) override;
};
