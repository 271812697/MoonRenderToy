/**
* @project: erload
* @author: erload Tech.
* @licence: MIT
*/

#pragma once

#include <Audio/Resources/Loaders/SoundLoader.h>

#include "Core/ResourceManagement/AResourceManager.h"

namespace Core::ResourceManagement
{
	/**
	* ResourceManager of sounds
	*/
	class SoundManager : public AResourceManager<Audio::Resources::Sound>
	{
	public:
		/**
		* Create the resource identified by the given path
		* @param p_path
		*/
		virtual Audio::Resources::Sound* CreateResource(const std::string& p_path) override;

		/**
		* Destroy the given resource
		* @param p_resource
		*/
		virtual void DestroyResource(Audio::Resources::Sound* p_resource) override;

		/**
		* Reload the given resource
		* @param p_resource
		* @param p_path
		*/
		virtual void ReloadResource(Audio::Resources::Sound* p_resource, const std::string& p_path) override;
	};
}