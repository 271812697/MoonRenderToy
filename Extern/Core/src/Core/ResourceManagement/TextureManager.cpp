
#include "Core/ResourceManagement/TextureManager.h"
#include "Rendering/Settings/DriverSettings.h"
#include"../../tools/IniFile.h"

std::tuple<Rendering::Settings::ETextureFilteringMode, Rendering::Settings::ETextureFilteringMode, bool> GetAssetMetadata(const std::string& p_path)
{
	auto metaFile = Tools::Filesystem::IniFile(p_path + ".meta");

	auto min = metaFile.GetOrDefault("MIN_FILTER", static_cast<int>(Rendering::Settings::ETextureFilteringMode::LINEAR_MIPMAP_LINEAR));
	auto mag = metaFile.GetOrDefault("MAG_FILTER", static_cast<int>(Rendering::Settings::ETextureFilteringMode::LINEAR));
	auto mipmap = metaFile.GetOrDefault("ENABLE_MIPMAPPING", true);

	return { static_cast<Rendering::Settings::ETextureFilteringMode>(min), static_cast<Rendering::Settings::ETextureFilteringMode>(mag), mipmap };
}

Rendering::Resources::Texture* Core::ResourceManagement::TextureManager::CreateResource(const std::string & p_path)
{
	std::string realPath = GetRealPath(p_path);

	auto [min, mag, mipmap] = GetAssetMetadata(realPath);

	Rendering::Resources::Texture* texture = Rendering::Resources::Loaders::TextureLoader::Create(realPath, min, mag, mipmap);
	if (texture)
		*reinterpret_cast<std::string*>(reinterpret_cast<char*>(texture) + offsetof(Rendering::Resources::Texture, path)) = p_path; // Force the resource path to fit the given path

	return texture;
}

void Core::ResourceManagement::TextureManager::DestroyResource(Rendering::Resources::Texture* p_resource)
{
	Rendering::Resources::Loaders::TextureLoader::Destroy(p_resource);
}

void Core::ResourceManagement::TextureManager::ReloadResource(Rendering::Resources::Texture* p_resource, const std::string& p_path)
{
	std::string realPath = GetRealPath(p_path);

	auto [min, mag, mipmap] = GetAssetMetadata(realPath);

	Rendering::Resources::Loaders::TextureLoader::Reload(*p_resource, realPath, min, mag, mipmap);
}
