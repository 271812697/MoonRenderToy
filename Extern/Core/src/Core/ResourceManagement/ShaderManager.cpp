#include "Core/ResourceManagement/ShaderManager.h"

Rendering::Resources::Shader* Core::ResourceManagement::ShaderManager::CreateResource(const std::string& p_path)
{
	std::string realPath = GetRealPath(p_path);
	auto pathParserCallback = std::bind(&Core::ResourceManagement::ShaderManager::GetRealPath, this, std::placeholders::_1);
	Rendering::Resources::Shader* shader = Rendering::Resources::Loaders::ShaderLoader::Create(realPath, pathParserCallback);
	if (shader)
		*reinterpret_cast<std::string*>(reinterpret_cast<char*>(shader) + offsetof(Rendering::Resources::Shader, path)) = p_path; // Force the resource path to fit the given path
	return shader;
}

::Rendering::Resources::Shader* Core::ResourceManagement::ShaderManager::CreateFromSource(const std::string& p_source)
{
	auto pathParserCallback = std::bind(&Core::ResourceManagement::ShaderManager::GetRealPath, this, std::placeholders::_1);
	Rendering::Resources::Shader* shader = Rendering::Resources::Loaders::ShaderLoader::CreateFromSource(p_source, pathParserCallback);
	return shader;
}

void Core::ResourceManagement::ShaderManager::DestroyResource(Rendering::Resources::Shader* p_resource)
{
	Rendering::Resources::Loaders::ShaderLoader::Destroy(p_resource);
}

void Core::ResourceManagement::ShaderManager::ReloadResource(Rendering::Resources::Shader* p_resource, const std::string& p_path)
{
	auto pathParserCallback = std::bind(&Core::ResourceManagement::ShaderManager::GetRealPath, this, std::placeholders::_1);
	Rendering::Resources::Loaders::ShaderLoader::Recompile(*p_resource, p_path, pathParserCallback);
}
