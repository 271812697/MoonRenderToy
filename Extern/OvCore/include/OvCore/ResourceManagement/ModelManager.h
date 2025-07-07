/**
* @project: Overload
* @author: Overload Tech.
* @licence: MIT
*/

#pragma once

#include <OvRendering/Resources/Loaders/ModelLoader.h>

#include "OvCore/ResourceManagement/AResourceManager.h"
#include "OvMaths/FVector2.h"

namespace OvCore::ResourceManagement
{
	/**
	* ResourceManager of models
	*/
	class ModelManager : public AResourceManager<OvRendering::Resources::Model>
	{
	public:
		/**
		* Create the resource identified by the given path
		* @param p_path
		*/
		virtual OvRendering::Resources::Model* CreateResource(const std::string& p_path) override;

		/**
		* Destroy the given resource
		* @param p_resource
		*/
		virtual void DestroyResource(OvRendering::Resources::Model* p_resource) override;

		/**
		* Reload the given resource
		* @param p_resource
		* @param p_path
		*/
		virtual void ReloadResource(OvRendering::Resources::Model* p_resource, const std::string& p_path) override;
		OvRendering::Resources::Model* LoadFromMemory(const std::vector<float>& v, const std::vector<unsigned int>& i);
		OvRendering::Resources::Model* LoadFromMemory(const std::vector<OvMaths::FVector3>& vertex, const std::vector<OvMaths::FVector3>& normal, const std::vector<OvMaths::FVector2>& uv, const std::vector<unsigned int>& i);
		OvRendering::Resources::Model* LoadFromMemory(const std::string& name, const std::vector<OvMaths::FVector3>& vertex, const std::vector<OvMaths::FVector3>& normal, const std::vector<OvMaths::FVector2>& uv, const std::vector<unsigned int>& i);

	};
}