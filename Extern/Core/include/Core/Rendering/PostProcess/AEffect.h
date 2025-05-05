/**
* @project: erload
* @author: erload Tech.
* @licence: MIT
*/

#pragma once

#include <Rendering/Core/ARenderPass.h>
#include <Rendering/Data/Material.h>
#include <Rendering/Core/CompositeRenderer.h>

namespace Core::Rendering::PostProcess
{
	/**
	* Base effect settings structure
	*/
	struct EffectSettings
	{
		bool enabled = true;
	};

	/**
	* Base class for any post-processing effect
	*/
	class AEffect
	{
	public:
		/**
		* Constructor of the effect class
		* @param p_renderer
		*/
		AEffect(::Rendering::Core::CompositeRenderer& p_renderer);

		/**
		* Returns true if the effect is applicable with the given settings.
		* If the effect is not applicable, it will be skipped by the post processing render pass
		* @param p_settings
		*/
		virtual bool IsApplicable(const EffectSettings& p_settings) const;

		/**
		* Draw the effect
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
		) = 0;

	protected:
		::Rendering::Core::CompositeRenderer& m_renderer;
	};
}
