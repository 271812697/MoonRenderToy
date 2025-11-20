#pragma once
#include <Rendering/Resources/Loaders/ModelLoader.h>
#include "Core/ResourceManagement/AResourceManager.h"
#include "Maths/FVector2.h"

namespace Core::ResourceManagement
{
	/**
	* ResourceManager of models
	*/
	class ModelManager : public AResourceManager<::Rendering::Resources::Model>
	{
	public:
		/**
		* Create the resource identified by the given path
		* @param p_path
		*/
		virtual ::Rendering::Resources::Model* CreateResource(const std::string& p_path) override;

		/**
		* Destroy the given resource
		* @param p_resource
		*/
		virtual void DestroyResource(::Rendering::Resources::Model* p_resource) override;

		/**
		* Reload the given resource
		* @param p_resource
		* @param p_path
		*/
		virtual void ReloadResource(::Rendering::Resources::Model* p_resource, const std::string& p_path) override;
		::Rendering::Resources::Model* LoadFromMemory(const std::string& name, const std::vector<float>& v, const std::vector<unsigned int>& i);

		::Rendering::Resources::Model* LoadFromMemory(const std::vector<Maths::FVector3>& vertex, const std::vector<Maths::FVector3>& normal, const std::vector<Maths::FVector2>& uv, const std::vector<unsigned int>& i);
		::Rendering::Resources::Model* LoadFromMemory(const std::string& name, const std::vector<Maths::FVector3>& vertex, const std::vector<Maths::FVector3>& normal, const std::vector<Maths::FVector2>& uv, const std::vector<unsigned int>& i);
		::Rendering::Resources::Model* LoadFromMemory(const std::string& name, const std::vector<Maths::FVector3>& vertex, const std::vector<unsigned int>& i);
	};
}