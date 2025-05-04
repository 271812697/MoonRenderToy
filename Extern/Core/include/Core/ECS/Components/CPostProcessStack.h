/**
* @project: erload
* @author: erload Tech.
* @licence: MIT
*/

#pragma once

#include "AComponent.h"

#include <Core/Rendering/PostProcess/PostProcessStack.h>
#include <Core/Rendering/PostProcess/BloomEffect.h>
#include <Core/Rendering/PostProcess/FXAAEffect.h>
#include <Core/Rendering/PostProcess/TonemappingEffect.h>
#include <Core/Rendering/PostProcess/AutoExposureEffect.h>

namespace Core::ECS { class Actor; }

namespace Core::ECS::Components
{
	/**
	* A component that handle a post process stack
	*/
	class CPostProcessStack : public AComponent
	{
	public:
		/**
		* Constructor
		* @param p_owner
		*/
		CPostProcessStack(ECS::Actor& p_owner);

		/**
		* Returns the name of the component
		*/
		std::string GetName() override;

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

		/**
		* Returns the post process setting stack
		*/
		const Core::Rendering::PostProcess::PostProcessStack& GetStack();

		/**
		* Returns the bloom settings
		*/
		const Core::Rendering::PostProcess::BloomSettings& GetBloomSettings() const;

		/**
		* Returns the auto-exposure settings
		*/
		const Core::Rendering::PostProcess::AutoExposureSettings& GetAutoExposureSettings() const;

		/**
		* Returns the tonemapping settings
		*/
		const Core::Rendering::PostProcess::TonemappingSettings& GetTonemappingSettings() const;

		/**
		* Returns the FXAA settings
		*/
		const Core::Rendering::PostProcess::FXAASettings& GetFXAASettings() const;

		/**
		* Sets the bloom settings
		* * @param p_settings
		*/
		void SetBloomSettings(const Core::Rendering::PostProcess::BloomSettings& p_settings);

		/**
		* Sets the auto exposure settings
		* * @param p_settings
		*/
		void SetAutoExposureSettings(const Core::Rendering::PostProcess::AutoExposureSettings& p_settings);

		/**
		* Sets the tonemapping settings
		* @param p_settings
		*/
		void SetTonemappingSettings(const Core::Rendering::PostProcess::TonemappingSettings& p_settings);

		/**
		* Sets the FXAA settings
		* @param p_settings
		*/
		void SetFXAASettings(const Core::Rendering::PostProcess::FXAASettings& p_settings);

	private:
		Core::Rendering::PostProcess::PostProcessStack m_settings;
	};
}