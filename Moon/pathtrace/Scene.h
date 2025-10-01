#pragma once
#include <string>
#include <vector>
#include <map>
#include "EnvironmentMap.h"
#include "Material.h"
#include "RendererOptions.h"
#include "Mesh.h"
#include "Trace.h"
#include "RadeonRays/bvh_translator.h"
namespace PathTrace
{
	class Camera;
	class Mesh;
	class Texture;
	class Material;
	class Renderer;
	struct Indices
	{
		int x, y, z;
	};

	class Scene
	{
	public:
		Scene();
		~Scene();

		int AddMesh(const std::string& filename);
		int AddTexture(const std::string& filename);
		int AddMaterial(const Material& material);
		int AddMeshInstance(const MeshInstance& meshInstance);
		int AddLight(const Light& light);

		void AddCamera(Vec3 eye, Vec3 lookat, float fov);
		void AddEnvMap(const std::string& filename);
		bool IntersectionByScreen(float x, float y, Vec3& p);
		Vec3 PathTrace(Vec3 origin, Vec3 direction);
		Camera* getCamera();
		// Instances
		std::vector<MeshInstance>& getMeshInstances();
		std::vector<std::vector<int>>& getMeshInstancesTree();
		std::vector<int>& getMeshInstancesRoots();
		std::vector<Light>& getLights();
		RadeonRays::BvhTranslator& getBvhTranslator();
		std::vector<Indices>& getVertIndices();
		std::vector<Vec4>& getVerticesUVX(); // Vertex + texture Coord (u/s)
		std::vector<Vec4>& getNormalsUVY(); // Normal + texture Coord (v/t)
		std::vector<Mat4>& getTransforms();
		std::vector<Texture*>& getTextures();
		std::vector<Material>& getMaterials();
		std::vector<Mesh*>& getMeshes();
		RenderOptions& getRenderOptions();
		EnvironmentMap* getEnvironmentMap();
		RadeonRays::bbox& getBBox();
		void setPath(const std::string& p);
		void setDirty(bool flag);
		void ProcessScene();
		void RebuildInstances();
		void Save();

	private:
		friend Renderer;
		// Options
		RenderOptions renderOptions;
		// Meshes
		std::vector<Mesh*> meshes;
		// Scene Mesh Data 
		std::vector<Indices> vertIndices;
		std::vector<Vec4> verticesUVX; // Vertex + texture Coord (u/s)
		std::vector<Vec4> normalsUVY; // Normal + texture Coord (v/t)
		std::vector<Mat4> transforms;

		// Materials
		std::vector<Material> materials;
		// Instances
		std::vector<MeshInstance> meshInstances;
		std::vector<std::vector<int>>meshInstancesTree;
		std::vector<int>meshInstancesRoots;
		int selectMeshInstance = -1;
		// Lights
		std::vector<Light> lights;
		// Environment Map
		EnvironmentMap* envMap;
		// Camera
		Camera* mCamera;
		// Bvh
		RadeonRays::BvhTranslator bvhTranslator; // Produces a flat bvh array for GPU consumption
		RadeonRays::bbox sceneBounds;
		// Texture Data
		std::vector<Texture*> textures;
		std::vector<unsigned char> textureMapsArray;
		bool initialized;
		bool dirty;
		// To check if scene elements need to be resent to GPU
		bool instancesModified = false;
		bool envMapModified = false;
		std::string path;
	private:
		RadeonRays::Bvh* sceneBvh;
		void createBLAS();
		void createTLAS();
	};
}
