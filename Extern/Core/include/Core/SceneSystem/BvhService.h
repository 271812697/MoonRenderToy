#pragma once
#include <string>
#include <Rendering/Geometry/bvh.h>
namespace Rendering::Resources {
	class Mesh;
}
namespace Core::Resources {
	class Material;
}
namespace Core::SceneSystem
{
	class Scene;
	struct MeshInstance
	{

		MeshInstance(std::string name, int meshId, const Maths::FMatrix4& xform, int matId)
			: name(name)
			, meshID(meshId)
			, transform(xform), localform(xform)
			, materialID(matId)
		{
			parentID = -1;
		}
		~MeshInstance() {}

		Maths::FMatrix4 transform;
		Maths::FMatrix4 localform;
		std::string name;

		int materialID;
		int meshID;
		int parentID;
	};
	struct Indices
	{
		int x, y, z;
	};
	class Material
	{
	public:
		Material()
		{
			baseColor = Maths::FVector3(1.0f, 1.0f, 1.0f);
			//各向异性
			anisotropic = 0.0f;

			emission = Maths::FVector3(0.0f, 0.0f, 0.0f);
			// padding1

			metallic = 0.0f;
			roughness = 0.5f;
			subsurface = 0.0f;
			specularTint = 0.0f;

			sheen = 0.0f;
			sheenTint = 0.0f;
			clearcoat = 0.0f;
			clearcoatGloss = 0.0f;

			specTrans = 0.0f;
			ior = 1.5f;
			//介质类型
			mediumType = 0.0f;
			mediumDensity = 0.0f;

			mediumColor = Maths::FVector3(1.0f, 1.0f, 1.0f);
			//各向同性
			mediumAnisotropy = 0.0f;

			baseColorTexId = -1.0f;
			metallicRoughnessTexID = -1.0f;
			normalmapTexID = -1.0f;
			emissionmapTexID = -1.0f;

			opacity = 1.0f;
			alphaMode = 0.0f;
			alphaCutoff = 0.0f;
			// padding2
		};

		Maths::FVector3 baseColor;
		float anisotropic;

		Maths::FVector3 emission;
		float ax;

		float metallic;
		float roughness;
		float subsurface;
		float specularTint;

		float sheen;
		float sheenTint;
		float clearcoat;
		float clearcoatGloss;

		float specTrans;
		float ior;
		float mediumType;
		float mediumDensity;

		Maths::FVector3 mediumColor;
		float mediumAnisotropy;

		float baseColorTexId;
		float metallicRoughnessTexID;
		float normalmapTexID;
		float emissionmapTexID;

		float opacity;
		float alphaMode;
		float alphaCutoff;
		float ay;
	};
	enum LightType
	{
		RectLight,
		SphereLight,
		DistantLight
	};

	struct Light
	{
		Maths::FVector3 position;
		Maths::FVector3 emission;
		Maths::FVector3 u;
		Maths::FVector3 v;
		float radius;
		float area;
		float type;
	};
	class BvhService {
	public:
		BvhService() = default;
		struct Node
		{
			Maths::FVector3 bboxmin;
			Maths::FVector3 bboxmax;
			Maths::FVector3 LRLeaf;
		};
		void AddMaterial(::Core::Resources::Material* material);
		void ProcessBLAS();
		void ProcessTLAS();
		int ProcessBLASNodes(const ::Rendering::Geometry::Bvh::Node* node);
		int ProcessTLASNodes(const ::Rendering::Geometry::Bvh::Node* node);

		void UpdateTLAS(const ::Rendering::Geometry::Bvh* topLevelBvh, const std::vector<MeshInstance>& instances);
		void Process(const ::Rendering::Geometry::Bvh* topLevelBvh, const std::vector<::Rendering::Resources::Mesh*>& sceneMeshes, const std::vector<MeshInstance>& instances);
		void Clear();
		~BvhService();
	public:
		friend class Scene;
		::Rendering::Geometry::Bvh* m_sceneBvh = nullptr;
		int topLevelIndex = 0;
		int nodeTexWidth;
		std::vector<Node> nodes;
		int curNode = 0;
		int curTriIndex = 0;
		std::vector<int> bvhRootStartIndices;
		std::vector<MeshInstance> meshInstances;
		std::vector<::Rendering::Resources::Mesh*> meshes;
		const ::Rendering::Geometry::Bvh* topLevelBvh;

		// Scene Mesh Data 
		std::vector<Indices> vertIndices;
		std::vector<Maths::FVector4> verticesUVX; // Vertex + texture Coord (u/s)
		std::vector<Maths::FVector4> normalsUVY; // Normal + texture Coord (v/t)
		std::vector<Maths::FMatrix4> transforms;

		// Materials
		std::vector<Material> materials;

		// Lights
		std::vector<Light> lights;
	};

}