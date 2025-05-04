/**
* @project: erload
* @author: erload Tech.
* @licence: MIT
*/

#pragma once

#include <Core/Rendering/PostProcess/AEffect.h>
#include <Rendering/Data/Material.h>

namespace Core::Rendering::PostProcess
{
	/**
	* Settings for the FXAA effect
	*/
	struct FXAASettings : public EffectSettings {};

	/**
	* Post-processing effect that applies FXAA (Fast Approximate Anti-Aliasing).
	*/
	class FXAAEffect : public AEffect
	{
	public:
		/**
		* Constructor of the FXAAEffect class
		* @param p_renderer
		*/
		FXAAEffect(::Rendering::Core::CompositeRenderer& p_renderer);

		/**
		* Render the FXAA effect
		* @note: make sure the effect is applicable before calling this method
		* @param p_pso
		* @param p_src
		* @param p_dst
		* @param p_settings
		*/
		virtual void Draw(
			::Rendering::Data::PipelineState p_pso,
			::Rendering::HAL::Framebuffer& p_src,
			::Rendering::HAL::Framebuffer& p_dst,
			const EffectSettings& p_settings
		) override;

	private:
		::Rendering::Data::Material m_material;
	};
}
