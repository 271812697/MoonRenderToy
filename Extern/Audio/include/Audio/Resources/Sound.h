/**
* @project: erload
* @author: erload Tech.
* @licence: MIT
*/

#pragma once

#include <string>
#include <memory>

namespace SoLoud
{
	class Wav;
}

namespace Audio::Resources
{
	namespace Loaders { class SoundLoader; }

	/**
	* Playable sound
	*/
	class Sound
	{
		friend class Loaders::SoundLoader;

	private:
		Sound(const std::string& p_path, std::unique_ptr<SoLoud::Wav>&& p_audioData);
		virtual ~Sound() = default;

	public:
		const std::string path;
		std::unique_ptr<SoLoud::Wav> audioData;
	};
}