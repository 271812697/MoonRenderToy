/**
* @project: erload
* @author: erload Tech.
* @licence: MIT
*/

#include <tracy/Tracy.hpp>

#include <Core/ECS/Components/CPostProcessStack.h>
#include <Core/Rendering/PostProcessRenderPass.h>
#include <Core/Global/ServiceLocator.h>
#include <Core/Rendering/FramebufferUtil.h>
#include <Core/Rendering/SceneRenderer.h>
#include <Core/ResourceManagement/ShaderManager.h>

#include <Rendering/Core/CompositeRenderer.h>
#include <Rendering/HAL/Profiling.h>

Core::Rendering::PostProcessRenderPass::PostProcessRenderPass(::Rendering::Core::CompositeRenderer& p_renderer) :
	::Rendering::Core::ARenderPass(p_renderer),
	m_pingPongBuffers{
		::Rendering::HAL::Framebuffer{"PostProcessBlitPingPong0"},
		::Rendering::HAL::Framebuffer{"PostProcessBlitPingPong1"}
	}
{
	for (auto& buffer : m_pingPongBuffers)
	{
		Core::Rendering::FramebufferUtil::SetupFramebuffer(
			buffer, 1, 1, false, false, false
		);
	}

	m_blitMaterial.SetShader(OVSERVICE(Core::ResourceManagement::ShaderManager)[":Shaders\\PostProcess\\Blit.ovfx"]);

	// Instantiate available effects
	m_effects.reserve(4);
	m_effects.push_back(std::make_unique<Core::Rendering::PostProcess::BloomEffect>(p_renderer));
	m_effects.push_back(std::make_unique<Core::Rendering::PostProcess::AutoExposureEffect>(p_renderer));
	m_effects.push_back(std::make_unique<Core::Rendering::PostProcess::TonemappingEffect>(p_renderer));
	m_effects.push_back(std::make_unique<Core::Rendering::PostProcess::FXAAEffect>(p_renderer));
}

Tools::Utils::OptRef<const Core::Rendering::PostProcess::PostProcessStack> FindPostProcessStack(Core::SceneSystem::Scene& p_scene)
{
	auto& postProcessStacks = p_scene.GetFastAccessComponents().postProcessStacks;

	for (auto postProcessStack : postProcessStacks)
	{
		if (postProcessStack && postProcessStack->owner.IsActive())
		{
			return postProcessStack->GetStack();
		}
	}

	return std::nullopt;
}

void Core::Rendering::PostProcessRenderPass::Draw(::Rendering::Data::PipelineState p_pso)
{
	ZoneScoped;
	TracyGpuZone("PostProcessRenderPass");

	auto& sceneDescriptor = m_renderer.GetDescriptor<Core::Rendering::SceneRenderer::SceneDescriptor>();
	auto& scene = sceneDescriptor.scene;

	if (auto stack = FindPostProcessStack(scene))
	{
		uint32_t passIndex = 0;

		auto& framebuffer = m_renderer.GetFrameDescriptor().outputBuffer.value();

		const uint64_t kPingPongBufferSize = m_pingPongBuffers.size();

		m_renderer.Blit(p_pso, framebuffer, m_pingPongBuffers[0], m_blitMaterial);

		for (auto& effect : m_effects)
		{
			const auto& settings = stack->Get(typeid(*effect));

			if (effect && effect->IsApplicable(settings))
			{
				effect->Draw(
					p_pso,
					m_pingPongBuffers[passIndex % kPingPongBufferSize],
					m_pingPongBuffers[(passIndex + 1) % kPingPongBufferSize],
					settings
				);

				++passIndex;
			}
		}

		const uint64_t lastIndex = passIndex % kPingPongBufferSize;

		m_renderer.Blit(p_pso, m_pingPongBuffers[lastIndex], framebuffer, m_blitMaterial);
	}
}
