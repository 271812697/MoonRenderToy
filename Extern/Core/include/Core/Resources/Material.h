/**
* @project: erload
* @author: erload Tech.
* @licence: MIT
*/

#pragma once

#include <any>
#include <map>

#include <Rendering/Data/Material.h>

#include "Core/API/ISerializable.h"

namespace Core::Resources
{
	/**
	* Engine material, which inherits from Rendering Material and adds serialization capabilities
	*/
	class Material : public ::Rendering::Data::Material, public API::ISerializable
	{
	public:
		/**
		* Serialize the material
		* @param p_doc
		* @param p_node
		*/
		virtual void OnSerialize(tinyxml2::XMLDocument& p_doc, tinyxml2::XMLNode* p_node) override;

		/**
		* Deserialize the material
		* @param p_doc
		* @param p_node
		*/
		virtual void OnDeserialize(tinyxml2::XMLDocument& p_doc, tinyxml2::XMLNode* p_node) override;

		const std::string path;
	};
}
