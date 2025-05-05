/**
* @project: erload
* @author: erload Tech.
* @licence: MIT
*/

#include <Core/Rendering/PostProcess/FXAAEffect.h>
#include <Core/Global/ServiceLocator.h>
#include <Core/ResourceManagement/ShaderManager.h>

Core::Rendering::PostProcess::FXAAEffect::FXAAEffect(::Rendering::Core::CompositeRenderer& p_renderer) : AEffect(p_renderer)
{
	m_material.SetShader(OVSERVICE(Core::ResourceManagement::ShaderManager)[":Shaders\\PostProcess\\FXAA.ovfx"]);
}

void Core::Rendering::PostProcess::FXAAEffect::Draw(
	::Rendering::Data::PipelineState p_pso,
	::Rendering::HAL::Framebuffer& p_src,
	::Rendering::HAL::Framebuffer& p_dst,
	const EffectSettings& p_settings
)
{
	m_renderer.Blit(p_pso, p_src, p_dst, m_material);
}
