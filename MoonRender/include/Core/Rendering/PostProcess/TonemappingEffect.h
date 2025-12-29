#pragma once
#include <chrono>
#include <Core/Rendering/PostProcess/AEffect.h>
#include <Rendering/Data/Material.h>

namespace Core::Rendering::PostProcess
{
	/**
	* Enum of available tonemapping modes
	*/
	enum class ETonemappingMode
	{
		NEUTRAL = 0,
		REINHARD = 1,
		REINHARD_JODIE = 2,
		UNCHARTED2 = 3,
		UNCHARTED2_FILMIC = 4,
		ACES = 5
	};

	/**
	* Tonemapping effect settings
	*/
	struct TonemappingSettings : public Core::Rendering::PostProcess::EffectSettings
	{
		float exposure = 1.0f;
		ETonemappingMode mode = ETonemappingMode::NEUTRAL;
		bool gammaCorrection = true;
	};

	/**
	* Post-processing effect that applies Tonemapping
	* Remap HDR values to the LDR range.
	*/
	class TonemappingEffect : public AEffect
	{
	public:
		/**
		* Constructor of the TonemappingEffect class
		* @param p_renderer
		*/
		TonemappingEffect(::Rendering::Core::CompositeRenderer& p_renderer);

		/**
		* Render the Tonemapping effect
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
		::Rendering::Data::Material m_tonemappingMaterial;
	};
}