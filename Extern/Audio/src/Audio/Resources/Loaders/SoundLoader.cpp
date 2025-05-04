/**
* @project: erload
* @author: erload Tech.
* @licence: MIT
*/

#include <soloud_wav.h>

#include <Audio/Resources/Loaders/SoundLoader.h>

Audio::Resources::Sound* Audio::Resources::Loaders::SoundLoader::Create(const std::string& p_filepath)
{
	std::unique_ptr<SoLoud::Wav> wav = std::make_unique<SoLoud::Wav>();
	wav->load(p_filepath.c_str());
	return new Sound(p_filepath, std::move(wav));
}

void Audio::Resources::Loaders::SoundLoader::Reload(Sound& p_sound, const std::string& p_path)
{
	*const_cast<std::string*>(&p_sound.path) = p_path;
	p_sound.audioData->load(p_path.c_str());
}

bool Audio::Resources::Loaders::SoundLoader::Destroy(Sound*& p_soundInstance)
{
	if (p_soundInstance)
	{
		delete p_soundInstance;
		p_soundInstance = nullptr;

		return true;
	}

	return false;
}
