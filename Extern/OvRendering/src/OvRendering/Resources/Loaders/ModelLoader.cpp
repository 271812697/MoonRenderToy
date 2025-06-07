/**
* @project: Overload
* @author: Overload Tech.
* @licence: MIT
*/

#include "OvRendering/Resources/Loaders/ModelLoader.h"

OvRendering::Resources::Parsers::AssimpParser OvRendering::Resources::Loaders::ModelLoader::__ASSIMP;

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
	result->m_meshes.push_back(new Mesh(vertices, indices, 0));
	result->m_materialNames.push_back("Default");
	result->ComputeBoundingSphere();
	return result;

}