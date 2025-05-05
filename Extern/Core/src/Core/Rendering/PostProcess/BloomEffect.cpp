/**
* @project: erload
* @author: erload Tech.
* @licence: MIT
*/

#include <Core/Global/ServiceLocator.h>
#include <Core/Rendering/FramebufferUtil.h>
#include <Core/Rendering/PostProcess/BloomEffect.h>
#include <Core/ResourceManagement/ShaderManager.h>

Core::Rendering::PostProcess::BloomEffect::BloomEffect(::Rendering::Core::CompositeRenderer& p_renderer) :
	AEffect(p_renderer),
	m_bloomPingPong{
		::Rendering::HAL::Framebuffer{"BloomPingPong0"},
		::Rendering::HAL::Framebuffer{"BloomPingPong1"}
	}
{
	for (auto& buffer : m_bloomPingPong)
	{
		FramebufferUtil::SetupFramebuffer(
			buffer, 1, 1, false, false, false
		);
	}

	auto& shaderManager = OVSERVICE(Core::ResourceManagement::ShaderManager);

	m_brightnessMaterial.SetShader(shaderManager[":Shaders\\PostProcess\\Brightness.ovfx"]);
	m_blurMaterial.SetShader(shaderManager[":Shaders\\PostProcess\\Blur.ovfx"]);
	m_bloomMaterial.SetShader(shaderManager[":Shaders\\PostProcess\\Bloom.ovfx"]);
}

bool Core::Rendering::PostProcess::BloomEffect::IsApplicable(const EffectSettings& p_settings) const
{
	auto& bloomSettings = static_cast<const BloomSettings&>(p_settings);

	return
		AEffect::IsApplicable(p_settings) &&
		bloomSettings.intensity > 0.0f;
}

void Core::Rendering::PostProcess::BloomEffect::Draw(
	::Rendering::Data::PipelineState p_pso,
	::Rendering::HAL::Framebuffer& p_src,
	::Rendering::HAL::Framebuffer& p_dst,
	const EffectSettings& p_settings
)
{
	const auto& bloomSettings = static_cast<const BloomSettings&>(p_settings);

	// Step 1: Extract bright spots from the source
	m_brightnessMaterial.SetProperty("_Threshold", bloomSettings.threshold, true);
	m_renderer.Blit(p_pso, p_src, m_bloomPingPong[0], m_brightnessMaterial);

	// Step 2: Apply gaussian blur on bright spots (horizontal and vertical)
	bool horizontal = true;

	for (int i = 0; i < bloomSettings.passes; ++i)
	{
		auto& currentSrc = horizontal ? m_bloomPingPong[0] : m_bloomPingPong[1];
		auto& currentDst = horizontal ? m_bloomPingPong[1] : m_bloomPingPong[0];

		m_blurMaterial.SetProperty("_Horizontal", horizontal ? true : false, true);
		m_blurMaterial.SetProperty("_BlurSize", bloomSettings.radius, true);
		m_blurMaterial.SetProperty("_KernelSize", bloomSettings.kernelSize, true);
		m_renderer.Blit(p_pso, currentSrc, currentDst, m_blurMaterial);

		horizontal = !horizontal;
	}

	// Step 3: Combine bloom with original framebuffer
	const auto bloomTex = m_bloomPingPong[0].GetAttachment<::Rendering::HAL::Texture>(::Rendering::Settings::EFramebufferAttachment::COLOR);
	m_bloomMaterial.SetProperty("_BloomTexture", &bloomTex.value(), true);
	m_bloomMaterial.SetProperty("_BloomIntensity", bloomSettings.intensity, true);
	m_renderer.Blit(p_pso, p_src, p_dst, m_bloomMaterial);
}
