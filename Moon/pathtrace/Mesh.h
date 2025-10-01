#pragma once
#include "MathUtil.h"
#include "RadeonRays/bbox.h"
#include <vector>
namespace RadeonRays {
	class Bvh;
}
namespace PathTrace
{
	class Mesh
	{
	public:
		Mesh();

		~Mesh();
		void GenVAO();
		void BuildBVH();
		bool LoadFromFile(const std::string& filename);
		void Draw();
		std::vector<float>PackData();
		RadeonRays::Bvh* getBvH();
		std::vector<Vec4>& getPosUvX();
		std::vector<Vec4>& getNorUvY();
		std::string getName();
		void setName(const std::string& name);
		RadeonRays::bbox& getBBox();
	private:
		//存储法线顶点坐标 uv分开存
		std::vector<Vec4> verticesUVX; // Vertex + texture Coord (u/s)
		std::vector<Vec4> normalsUVY;  // Normal + texture Coord (v/t)
		unsigned int vao;
		unsigned int vbop;
		unsigned int vbon;
		bool hasVAO = false;

		RadeonRays::Bvh* mBvh;
		std::string mName;
		RadeonRays::bbox meshBounds;
	
	};

	class MeshInstance
	{

	public:
		MeshInstance(std::string name, int meshId, Mat4 xform, int matId)
			: name(name)
			, meshID(meshId)
			, transform(xform), localform(xform)
			, materialID(matId)
		{
			parentID = -1;
		}
		~MeshInstance() {}

		Mat4 transform;
		Mat4 localform;
		std::string name;

		int materialID;
		int meshID;
		int parentID;
	};
}

