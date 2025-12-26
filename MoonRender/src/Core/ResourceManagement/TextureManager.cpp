#include <format>
#include <Core/ResourceManagement/TextureManager.h>
#include <Rendering/Settings/DriverSettings.h>
#include <Tools/Filesystem/IniFile.h>

namespace
{
	struct TextureMetaData
	{
		Rendering::Settings::ETextureFilteringMode minFilter;
		Rendering::Settings::ETextureFilteringMode magFilter;
		Rendering::Settings::ETextureWrapMode horizontalWrap;
		Rendering::Settings::ETextureWrapMode verticalWrap;
		bool generateMipmap;
	};

	TextureMetaData LoadTextureMetadata(const std::string_view p_filePath)
	{
		using namespace Rendering::Settings;
		using enum ETextureFilteringMode;
		using enum ETextureWrapMode;

		const auto metaFile = Tools::Filesystem::IniFile(std::format("{}.meta", p_filePath));

		return TextureMetaData{
			.minFilter = static_cast<ETextureFilteringMode>(metaFile.GetOrDefault("MIN_FILTER", static_cast<int>(LINEAR_MIPMAP_LINEAR))),
			.magFilter = static_cast<ETextureFilteringMode>(metaFile.GetOrDefault("MAG_FILTER", static_cast<int>(LINEAR))),
			.horizontalWrap = static_cast<ETextureWrapMode>(metaFile.GetOrDefault("HORIZONTAL_WRAP", static_cast<int>(REPEAT))),
			.verticalWrap = static_cast<ETextureWrapMode>(metaFile.GetOrDefault("VERTICAL_WRAP", static_cast<int>(REPEAT))),
			.generateMipmap = metaFile.GetOrDefault("ENABLE_MIPMAPPING", true)
		};
	}
}

Rendering::Resources::Texture* Core::ResourceManagement::TextureManager::CreateResource(const std::string& p_path)
{
	std::string realPath = GetRealPath(p_path);

	const auto metaData = LoadTextureMetadata(realPath);

	Rendering::Resources::Texture* texture = Rendering::Resources::Loaders::TextureLoader::Create(
		realPath,
		metaData.minFilter,
		metaData.magFilter,
		metaData.horizontalWrap,
		metaData.verticalWrap,
		metaData.generateMipmap
	);

	if (texture)
		*reinterpret_cast<std::string*>(reinterpret_cast<char*>(texture) + offsetof(Rendering::Resources::Texture, path)) = p_path; // Force the resource path to fit the given path

	return texture;
}

Rendering::Resources::Texture* Core::ResourceManagement::TextureManager::CreateFromMemory(const std::string& p_path, unsigned char* data, int w, int h)
{
	Rendering::Resources::Texture* texture = Rendering::Resources::Loaders::TextureLoader::CreateFromMemory(data, w, h, Rendering::Settings::ETextureFilteringMode::LINEAR_MIPMAP_LINEAR,
		Rendering::Settings::ETextureFilteringMode::LINEAR,
		Rendering::Settings::ETextureWrapMode::REPEAT,
		Rendering::Settings::ETextureWrapMode::REPEAT,
		true
	);
	if (texture)
		*reinterpret_cast<std::string*>(reinterpret_cast<char*>(texture) + offsetof(Rendering::Resources::Texture, path)) = p_path; // Force the resource path to fit the given path
	RegisterResource(p_path, texture);
	return texture;
}

void Core::ResourceManagement::TextureManager::DestroyResource(Rendering::Resources::Texture* p_resource)
{
	Rendering::Resources::Loaders::TextureLoader::Destroy(p_resource);
}

void Core::ResourceManagement::TextureManager::ReloadResource(Rendering::Resources::Texture* p_resource, const std::string& p_path)
{
	std::string realPath = GetRealPath(p_path);

	const auto metaData = LoadTextureMetadata(realPath);

	Rendering::Resources::Loaders::TextureLoader::Reload(
		*p_resource,
		realPath,
		metaData.minFilter,
		metaData.magFilter,
		metaData.horizontalWrap,
		metaData.verticalWrap,
		metaData.generateMipmap
	);
}
