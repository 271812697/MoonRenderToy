

#pragma once

#include "Core/ECS/Components/CLight.h"

namespace Core::ECS { class Actor; }

namespace Core::ECS::Components
{
	/**
	* A light that is localized, has attenuation and has a direction
	*/
	class CSpotLight : public CLight
	{
	public:
		/**
		* Constructor
		* @param p_owner
		*/
		CSpotLight(ECS::Actor& p_owner);

		/**
		* Returns the name of the component
		*/
		std::string GetName() override;

		/**
		* Returns the light constant
		*/
		float GetConstant() const;

		/**
		* Returns the light linear
		*/
		float GetLinear() const;

		/**
		* Returns the light quadratic
		*/
		float GetQuadratic() const;

		/**
		* Returns the light cutoff
		*/
		float GetCutoff() const;
        float GetRange()const;

		/**
		* Returns the light outercutoff
		*/
		float GetOuterCutoff() const;

		/**
		* Defines the light constant
		* @param p_constant
		*/
		void SetConstant(float p_constant);
        void SetRange(float p_range);


		/**
		* Defines the light linear
		* @param p_linear
		*/
		void SetLinear(float p_linear);

		/**
		* Defines the light quadratic
		* @param p_quadratic
		*/
		void SetQuadratic(float p_quadratic);

		/**
		* Defines the light cutoff
		* @param p_cutoff
		*/
		void SetCutoff(float p_cutoff);

		/**
		* Defines the light outercutoff
		* @param p_cutoff
		*/
		void SetOuterCutoff(float p_outerCutoff);

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