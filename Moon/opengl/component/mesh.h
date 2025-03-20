#pragma once
#include <vector>
#include <glm/glm.hpp>
#include "core/base.h"
#include "../asset/vao.h"
#include "../asset/buffer.h"
#include "component.h"

namespace component {

	enum class Primitive : uint8_t {
		Sphere, Cube, Plane, Quad2D, Torus, Capsule, Tetrahedron
	};

	class Mesh : public Component {
	public:
		struct Vertex {
			glm::vec3  position;
			glm::vec3  normal;
			glm::vec2  uv;
			glm::vec2  uv2;
			glm::vec3  tangent;
			glm::vec3  binormal;
			glm::ivec4 bone_id;  // 4 bones per vertex rule
			glm::vec4  bone_wt;  // the weight of each bone
		};

		static_assert(sizeof(Vertex) == 20 * sizeof(float) + 4 * sizeof(int));
		size_t n_verts, n_tris;

	private:
		friend class Model;
		asset_ref<asset::VAO> vao;
		asset_ref<asset::VBO> vbo;
		asset_ref<asset::IBO> ibo;

		void CreateSphere(float radius = 1.0f);
		void CreateCube(float size = 1.0f);
		void CreatePlane(float size = 10.0f);
		void Create2DQuad(float size = 1.0f);
		void CreateTorus(float R = 1.5f, float r = 0.5f);
		void CreateCapsule(float a = 2.0f, float r = 1.0f);
		void CreatePyramid(float s = 2.0f);
		void CreateBuffers(const std::vector<Vertex>& vertices, const std::vector<GLuint>& indices);

	public:
		Mesh(Primitive object);
		Mesh(asset_ref<asset::VAO> vao, size_t n_verts);
		Mesh(const std::vector<Vertex>& vertices, const std::vector<GLuint>& indices);
		Mesh(const asset_ref<Mesh>& mesh_asset);

		void Draw() const;
		static void DrawQuad();
		static void DrawGrid();

		// this field is only used by meshes that are loaded from external models
		mutable GLuint material_id;
		void SetMaterialID(GLuint mid) const;
	};

}
