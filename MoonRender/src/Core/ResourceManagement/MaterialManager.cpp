#include "Core/ResourceManagement/MaterialManager.h"

Core::Resources::Material * Core::ResourceManagement::MaterialManager::CreateResource(const std::string & p_path)
{
	std::string realPath = GetRealPath(p_path);

	Resources::Material* material = Core::Resources::Loaders::MaterialLoader::Create(realPath);
	if (material)
	{
		*reinterpret_cast<std::string*>(reinterpret_cast<char*>(material) + offsetof(Resources::Material, path)) = p_path; // Force the resource path to fit the given path
	}

	return material;
}

void Core::ResourceManagement::MaterialManager::DestroyResource(Core::Resources::Material * p_resource)
{
	Core::Resources::Loaders::MaterialLoader::Destroy(p_resource);
}

void Core::ResourceManagement::MaterialManager::ReloadResource(Core::Resources::Material* p_resource, const std::string& p_path)
{
	std::string realPath = GetRealPath(p_path);
	Core::Resources::Loaders::MaterialLoader::Reload(*p_resource, realPath);
}
