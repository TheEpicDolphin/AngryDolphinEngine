#pragma once

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <unordered_map>
#include <vector>

#include "renderer.h"

class OpenGLRenderer : IRenderer
{
public:
	void Initialize(int width, int height);

	void SetRenderableObject(UID id, RenderableObjectInfo info);

	void UnsetRenderableObject(UID id);

	bool RenderFrame();

	void Cleanup();

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

	typedef struct RenderableID {
		MaterialID material_id;
		MeshID mesh_id;
	} RenderableID;

	GLFWwindow* window_;
	std::unordered_map<MaterialID, MaterialBatch> material_batch_map_;
	std::unordered_map<UID, RenderableID> renderable_object_map_;

	static void DestroyWindow();

	static MeshBatch CreateMeshBatch(GLuint& vao, Material& material, Mesh& mesh);
};
