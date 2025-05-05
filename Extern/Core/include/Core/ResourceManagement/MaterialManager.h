/**
* @project: erload
* @author: erload Tech.
* @licence: MIT
*/

#pragma once

#include "Core/Resources/Loaders/MaterialLoader.h"
#include "Core/ResourceManagement/AResourceManager.h"

namespace Core::ResourceManagement
{
	/**
	* ResourceManager of materials
	*/
	class MaterialManager : public AResourceManager<Core::Resources::Material>
	{
	public:
		/**
		* Create the resource identified by the given path
		* @param p_path
		*/
		virtual Core::Resources::Material* CreateResource(const std::string & p_path) override;

		/**
		* Destroy the given resource
		* @param p_resource
		*/
		virtual void DestroyResource(Core::Resources::Material* p_resource) override;

		/**
		* Reload the given resource
		* @param p_resource
		* @param p_path
		*/
		virtual void ReloadResource(Core::Resources::Material* p_resource, const std::string& p_path) override;
	};
}