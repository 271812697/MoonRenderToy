/**
* @project: erload
* @author: erload Tech.
* @licence: MIT
*/


#include <Core/ECS/Components/CMaterialRenderer.h>
#include <Core/Rendering/ShadowRenderFeature.h>

#include <Debug/Logger.h>

#include <Rendering/Features/LightingRenderFeature.h>

constexpr uint8_t kMaxShadowMaps = 1;

Core::Rendering::ShadowRenderFeature::ShadowRenderFeature(::Rendering::Core::CompositeRenderer& p_renderer) :
	ARenderFeature(p_renderer)
{
}

void Core::Rendering::ShadowRenderFeature::OnBeforeDraw(::Rendering::Data::PipelineState& p_pso, const ::Rendering::Entities::Drawable& p_drawable)
{
	auto& material = p_drawable.material.value();

	if (material.IsShadowReceiver())
	{
		ASSERT(m_renderer.HasDescriptor<::Rendering::Features::LightingRenderFeature::LightingDescriptor>(), "Cannot find LightingDescriptor attached to this renderer");

		auto& lightDescriptor = m_renderer.GetDescriptor<::Rendering::Features::LightingRenderFeature::LightingDescriptor>();

		uint8_t lightIndex = 0;

		for (auto lightReference : lightDescriptor.lights)
		{
			const auto& light = lightReference.get();

			if (light.castShadows)
			{
				if (lightIndex < kMaxShadowMaps)
				{
					if (light.type == ::Rendering::Settings::ELightType::DIRECTIONAL)
					{
						const auto shadowTex = light.GetShadowBuffer().GetAttachment<::Rendering::HAL::Texture>(::Rendering::Settings::EFramebufferAttachment::DEPTH);
						material.SetProperty("_ShadowMap", &shadowTex.value(), true); // Single use material property
						material.SetProperty("_LightSpaceMatrix", light.GetLightSpaceMatrix());
						++lightIndex;
					}
				}
				else
				{
					OVLOG_WARNING("ShadowRenderFeature does not support more than one shadow casting directional light at the moment");
				}
			}
		}
	}
}

void Core::Rendering::ShadowRenderFeature::OnAfterDraw(::Rendering::Data::PipelineState& p_pso, const ::Rendering::Entities::Drawable& p_drawable)
{

}

