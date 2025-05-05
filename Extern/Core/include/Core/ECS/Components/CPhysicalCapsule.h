/**
* @project: erload
* @author: erload Tech.
* @licence: MIT
*/

#pragma once

#include "Core/ECS/Components/AComponent.h"

#include "Core/ECS/Components/CPhysicalObject.h"

namespace Core::ECS { class Actor; }

namespace Core::ECS::Components
{
	/**
	* Represent a physical object with a capsule hape
	*/
	class CPhysicalCapsule : public CPhysicalObject
	{
	public:
		/**
		* Constructor
		* @param p_owner
		*/
		CPhysicalCapsule(ECS::Actor& p_owner);

		/**
		* Returns the name of the component
		*/
		std::string GetName() override;

		/**
		* Defines the radius of the capsule shape
		* @param p_radius
		*/
		void SetRadius(float p_radius);

		/**
		* Defines the height of the capsule shape
		* @param p_height
		*/
		void SetHeight(float p_height);

		/**
		* Returns the radius of the capsule shape
		*/
		float GetRadius() const;

		/**
		* Returns the height of the capsule shape
		*/
		float GetHeight() const;

		/**
		* Serialize the component
		* @param p_doc
		* @param p_node
		*/
		virtual void OnSerialize(tinyxml2::XMLDocument& p_doc, tinyxml2::XMLNode* p_node) override;

		/**
		* Deserialize the component
		* @param p_doc
		* @param p_node
		*/
		virtual void OnDeserialize(tinyxml2::XMLDocument& p_doc, tinyxml2::XMLNode* p_node) override;

		/**
		* Defines how the component should be drawn in the inspector
		* @param p_root
		*/
		//virtual void OnInspector(UI::Internal::WidgetContainer& p_root) override;
	};
}