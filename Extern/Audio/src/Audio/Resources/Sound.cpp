/**
* @project: erload
* @author: erload Tech.
* @licence: MIT
*/

#include <Audio/Resources/Sound.h>

#include <soloud_wav.h>

Audio::Resources::Sound::Sound(const std::string& p_path, std::unique_ptr<SoLoud::Wav>&& p_audioData) :
	path(p_path),
	audioData(std::move(p_audioData))
{
}
