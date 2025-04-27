

#include "Core/ResourceManagement/SoundManager.h"

Audio::Resources::Sound* Core::ResourceManagement::SoundManager::CreateResource(const std::string& p_path)
{
	std::string realPath = GetRealPath(p_path);
	Audio::Resources::Sound* sound = Audio::Resources::Loaders::SoundLoader::Create(realPath);
	if (sound)
		*reinterpret_cast<std::string*>(reinterpret_cast<char*>(sound) + offsetof(Audio::Resources::Sound, path)) = p_path; // Force the resource path to fit the given path

	return sound;
}

void Core::ResourceManagement::SoundManager::DestroyResource(Audio::Resources::Sound* p_resource)
{
	Audio::Resources::Loaders::SoundLoader::Destroy(p_resource);
}

void Core::ResourceManagement::SoundManager::ReloadResource(Audio::Resources::Sound* p_resource, const std::string& p_path)
{
}
