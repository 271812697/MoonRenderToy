/**
* @project: erload
* @author: erload Tech.
* @licence: MIT
*/

#pragma once

#include <Core/Rendering/PostProcess/AEffect.h>
#include <Rendering/Data/Material.h>
#include <Rendering/HAL/Framebuffer.h>

namespace Core::Rendering::PostProcess
{
	/**
	* Settings for the BloomEffect
	*/
	struct BloomSettings : public EffectSettings
	{
		float threshold = 0.8f;
		float radius = 5.0f;
		int kernelSize = 6;
		float intensity = 0.6f;
		int passes = 10;
	};

	/**
	* Bloom post-processing effect:
	* This effect will make the bright parts of the image glow
	*/
	class BloomEffect : public AEffect
	{
	public:
		/**
		* Constructor of the BloomEffect class
		* @param p_renderer
		*/
		BloomEffect(::Rendering::Core::CompositeRenderer& p_renderer);

		/**
		* Returns true if the effect is applicable with the given settings.
		* If the effect is not applicable, it will be skipped by the post processing render pass
		* @param p_settings
		*/
		virtual bool IsApplicable(const EffectSettings& p_settings) const override;

		/**
		* Render the bloom effect
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
		std::array<::Rendering::HAL::Framebuffer, 2> m_bloomPingPong;
		::Rendering::Data::Material m_brightnessMaterial;
		::Rendering::Data::Material m_blurMaterial;
		::Rendering::Data::Material m_bloomMaterial;
	};
}