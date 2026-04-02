#include <tracy/Tracy.hpp>
#include <Core/ECS/Components/CMaterialRenderer.h>
#include <Core/Rendering/EngineDrawableDescriptor.h>
#include <Core/Rendering/SsaoRenderFeature.h>
#include "Rendering/Core/CompositeRenderer.h"
#include "Core/Rendering/GbufferPass.h"



Core::Rendering::SsaoRenderFeature::SsaoRenderFeature(
	::Rendering::Core::CompositeRenderer& p_renderer,
	::Rendering::Features::EFeatureExecutionPolicy p_executionPolicy
) :
	ARenderFeature(p_renderer, p_executionPolicy)
{
}


void Core::Rendering::SsaoRenderFeature::OnBeginFrame(const ::Rendering::Data::FrameDescriptor& p_frameDescriptor)
{

}

void Core::Rendering::SsaoRenderFeature::OnBeforeDraw(::Rendering::Data::PipelineState& p_pso, const ::Rendering::Entities::Drawable& p_drawable)
{
	ZoneScoped;
	auto& occlusionTex =m_renderer.GetPass<Core::Rendering::GbufferPass>("Gbuffer").GetGbufferData().occlusionBlur;
	auto& material = p_drawable.material.value();
	if ( material.HasProperty("occlusionTex"))
	{
		material.SetProperty("occlusionTex", occlusionTex.get());
	}
}
