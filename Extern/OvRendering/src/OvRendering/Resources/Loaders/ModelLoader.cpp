/**
* @project: Overload
* @author: Overload Tech.
* @licence: MIT
*/

#include "OvRendering/Resources/Loaders/ModelLoader.h"

OvRendering::Resources::Parsers::AssimpParser OvRendering::Resources::Loaders::ModelLoader::__ASSIMP;

inline unsigned GetIndex(void*& indexPointer, unsigned indexSize)
{
	if (indexSize == sizeof(unsigned short))
	{
		auto& p = (unsigned short*&)indexPointer;
		return *p++;
	}
	else
	{
		auto& p = (unsigned*&)indexPointer;
		return *p++;
	}
}
void GenerateTangents(void* vertexData, unsigned vertexSize, const void* indexData, unsigned indexSize, unsigned indexStart,
	unsigned indexCount, unsigned normalOffset, unsigned texCoordOffset, unsigned tangentOffset)
{
	// Tangent generation from
	// http://www.terathon.com/code/tangent.html
	unsigned minVertex = 0xFFFFFFFF;
	unsigned maxVertex = 0;
	auto* vertices = (unsigned char*)vertexData;

	auto* indexPointer = const_cast<void*>(indexData);
	for (unsigned i = indexStart; i < indexStart + indexCount; ++i)
	{
		unsigned v = GetIndex(indexPointer, indexSize);
		if (v < minVertex)
			minVertex = v;
		if (v > maxVertex)
			maxVertex = v;
	}

	unsigned vertexCount = maxVertex + 1;
	auto* tan1 = new OvMaths::FVector3[vertexCount * 2];
	OvMaths::FVector3* tan2 = tan1 + vertexCount;
	memset(tan1, 0, sizeof(OvMaths::FVector3) * vertexCount * 2);

	indexPointer = const_cast<void*>(indexData);
	for (unsigned i = indexStart; i < indexStart + indexCount; i += 3)
	{
		unsigned i1 = GetIndex(indexPointer, indexSize);
		unsigned i2 = GetIndex(indexPointer, indexSize);
		unsigned i3 = GetIndex(indexPointer, indexSize);

		const OvMaths::FVector3& v1 = *((OvMaths::FVector3*)(vertices + i1 * vertexSize));
		const OvMaths::FVector3& v2 = *((OvMaths::FVector3*)(vertices + i2 * vertexSize));
		const OvMaths::FVector3& v3 = *((OvMaths::FVector3*)(vertices + i3 * vertexSize));

		const OvMaths::FVector2& w1 = *((OvMaths::FVector2*)(vertices + i1 * vertexSize + texCoordOffset));
		const OvMaths::FVector2& w2 = *((OvMaths::FVector2*)(vertices + i2 * vertexSize + texCoordOffset));
		const OvMaths::FVector2& w3 = *((OvMaths::FVector2*)(vertices + i3 * vertexSize + texCoordOffset));

		float x1 = v2.x - v1.x;
		float x2 = v3.x - v1.x;
		float y1 = v2.y - v1.y;
		float y2 = v3.y - v1.y;
		float z1 = v2.z - v1.z;
		float z2 = v3.z - v1.z;

		float s1 = w2.x - w1.x;
		float s2 = w3.x - w1.x;
		float t1 = w2.y - w1.y;
		float t2 = w3.y - w1.y;

		float r = 1.0f / (s1 * t2 - s2 * t1);
		OvMaths::FVector3 sdir((t2 * x1 - t1 * x2) * r, (t2 * y1 - t1 * y2) * r, (t2 * z1 - t1 * z2) * r);
		OvMaths::FVector3 tdir((s1 * x2 - s2 * x1) * r, (s1 * y2 - s2 * y1) * r, (s1 * z2 - s2 * z1) * r);

		tan1[i1] += sdir;
		tan1[i2] += sdir;
		tan1[i3] += sdir;

		tan2[i1] += tdir;
		tan2[i2] += tdir;
		tan2[i3] += tdir;
	}

	for (unsigned i = minVertex; i <= maxVertex; i++)
	{
		const OvMaths::FVector3& n = *((OvMaths::FVector3*)(vertices + i * vertexSize + normalOffset));
		const OvMaths::FVector3& t = tan1[i];
		OvMaths::FVector3 xyz;
		float w;

		// Gram-Schmidt orthogonalize
		xyz = OvMaths::FVector3::Normalize((t - n * OvMaths::FVector3::Dot(n, t)));
		
		// Calculate handedness
		//w = n.CrossProduct(t).DotProduct(tan2[i]) < 0.0f ? -1.0f : 1.0f;

		OvMaths::FVector3& tangent = *((OvMaths::FVector3*)(vertices + i * vertexSize + tangentOffset));
		tangent = xyz;
	}

	delete[] tan1;
}


OvRendering::Resources::Model* OvRendering::Resources::Loaders::ModelLoader::Create(const std::string& p_filepath, Parsers::EModelParserFlags p_parserFlags)
{
	Model* result = new Model(p_filepath);

	if (__ASSIMP.LoadModel(p_filepath, result->m_meshes, result->m_materialNames, p_parserFlags))
	{
		result->ComputeBoundingSphere();
		return result;
	}

	delete result;

	return nullptr;
}

void OvRendering::Resources::Loaders::ModelLoader::Reload(Model& p_model, const std::string& p_filePath, Parsers::EModelParserFlags p_parserFlags)
{
	Model* newModel = Create(p_filePath, p_parserFlags);

	if (newModel)
	{
		p_model.m_meshes = newModel->m_meshes;
		p_model.m_materialNames = newModel->m_materialNames;
		p_model.m_boundingSphere = newModel->m_boundingSphere;
		newModel->m_meshes.clear();
		delete newModel;
	}
}

bool OvRendering::Resources::Loaders::ModelLoader::Destroy(Model*& p_modelInstance)
{
	if (p_modelInstance)
	{
		delete p_modelInstance;
		p_modelInstance = nullptr;

		return true;
	}

	return false;
}

OvRendering::Resources::Model* OvRendering::Resources::Loaders::ModelLoader::LoadFromMemory(const std::vector<float>& v, const std::vector<unsigned int>& i)
{
	Model* result = new Model("Memory");
	std::vector<Geometry::Vertex> vertices(v.size() / 8);
	std::vector<uint32_t> indices = i;
	for (int k = 0; k < vertices.size(); k++) {
		vertices[k].position[0] = v[8 * k];
		vertices[k].position[1] = v[8 * k + 1];
		vertices[k].position[2] = v[8 * k + 2];
		vertices[k].normals[0] = v[8 * k + 3];
		vertices[k].normals[1] = v[8 * k + 4];
		vertices[k].normals[2] = v[8 * k + 5];
		vertices[k].texCoords[0] = v[8 * k + 6];
		vertices[k].texCoords[1] = v[8 * k + 7];
	}
	GenerateTangents(vertices.data(),sizeof(Geometry::Vertex),indices.data(),4,0,indices.size(),20,12,32);
	result->m_meshes.push_back(new Mesh(vertices, indices, 0));
	result->m_materialNames.push_back("Default");
	result->ComputeBoundingSphere();
	return result;

}

OvRendering::Resources::Model* OvRendering::Resources::Loaders::ModelLoader::LoadFromMemory(const std::vector<OvMaths::FVector3>& vertex, const std::vector<OvMaths::FVector3>& normal, const std::vector<OvMaths::FVector2>& uv, const std::vector<unsigned int>& i)
{
	Model* result = new Model("Memory");
	std::vector<Geometry::Vertex> vertices(vertex.size());
	std::vector<uint32_t> indices = i;
	for (int k = 0; k < vertices.size(); k++) {
		vertices[k].position[0] = vertex[k].x;
		vertices[k].position[1] = vertex[k].y;
		vertices[k].position[2] = vertex[k].z;
		vertices[k].normals[0] = normal[k].x;
		vertices[k].normals[1] = normal[k].y;
		vertices[k].normals[2] = normal[k].z;
		vertices[k].texCoords[0] = uv[k].x;
		vertices[k].texCoords[1] = uv[k].y;
	}
	GenerateTangents(vertices.data(), sizeof(Geometry::Vertex), indices.data(), 4, 0, indices.size(), 20, 12, 32);
	result->m_meshes.push_back(new Mesh(vertices, indices, 0));
	result->m_materialNames.push_back("Default");
	result->ComputeBoundingSphere();
	return result;
}
