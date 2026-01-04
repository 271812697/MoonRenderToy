#pragma once
#include <vector>
#include <unordered_map>
#include <string>
#include "Rendering/Resources/Mesh.h"
namespace Rendering::Resources
{
	namespace Loaders { class ModelLoader; }
	class Model
	{
		friend class Loaders::ModelLoader;
	public:
		std::vector<Mesh*>& GetMeshes() ;
		std::vector<std::string>& GetMaterialNames() ;
		const Rendering::Geometry::BoundingSphere& GetBoundingSphere() const;
		const Rendering::Geometry::bbox& GetBoundingBox() ;
	
		Model(const std::string& p_path);
		~Model();
	private:
		void ComputeBoundingSphere();
		void ComputeBoundingBox();

	public:
		const std::string path;

	private:
		std::vector<Mesh*> m_meshes;
		std::vector<std::string> m_materialNames;
		Geometry::bbox m_boundingBox;
		Geometry::BoundingSphere m_boundingSphere;
	};
}