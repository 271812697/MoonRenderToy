/**
* @project: erload
* @author: erload Tech.
* @licence: MIT
*/

#pragma once

#include <string>

#include "Audio/Resources/Sound.h"

namespace Audio::Resources::Loaders
{
	/**
	* Handle the Mesh creation and destruction
	*/
	class SoundLoader
	{
	public:
		/**
		* Disabled constructor
		*/
		SoundLoader() = delete;

		/**
		* Create a sound
		* @param p_filepath
		*/
		static Sound* Create(const std::string& p_filepath);

		/**
		* Reload a sound
		* @param p_sound
		* @param p_path
		*/
		static void Reload(Sound& p_sound, const std::string& p_path);

		/**
		* Destroy a sound
		* @param p_soundInstance
		*/
		static bool Destroy(Sound*& p_soundInstance);
	};
}