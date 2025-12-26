#include <functional>
#include <ranges>
#include <tracy/Tracy.hpp>

#include <Rendering/Core/CompositeRenderer.h>
#include <Rendering/HAL/Profiling.h>

Rendering::Core::CompositeRenderer::CompositeRenderer(Context::Driver& p_driver)
	: ABaseRenderer(p_driver)
{
}

void Rendering::Core::CompositeRenderer::BeginFrame(const Data::FrameDescriptor& p_frameDescriptor)
{
	ZoneScoped;
	TracyGpuZone("BeginFrame");

	ABaseRenderer::BeginFrame(p_frameDescriptor);

	for (const auto& feature : m_features | std::views::values)
	{
		if (feature->IsEnabled())
		{
			feature->OnBeginFrame(p_frameDescriptor);
		}
	}

	for (const auto& pass : m_passes | std::views::values)
	{
		if (pass.second->IsEnabled())
		{
			pass.second->OnBeginFrame(p_frameDescriptor);
		}
	}
}

void Rendering::Core::CompositeRenderer::DrawFrame()
{
	ZoneScoped;
	TracyGpuZone("DrawFrame");
	auto pso = CreatePipelineState();
	for (const auto& pass : m_passes | std::views::values)
	{
		m_currentPass = pass.second.get();

		m_frameDescriptor.outputMsaaBuffer.value().Bind();
		SetViewport(0, 0, m_frameDescriptor.renderWidth, m_frameDescriptor.renderHeight);

		if (m_currentPass->IsEnabled())
		{
			m_currentPass->Draw(pso);
		}

		m_currentPass.reset();
	}
}

void Rendering::Core::CompositeRenderer::EndFrame()
{
	ZoneScoped;
	TracyGpuZone("EndFrame");

	for (const auto& pass : m_passes | std::views::values)
	{
		if (pass.second->IsEnabled())
		{
			pass.second->OnEndFrame();
		}
	}

	for (const auto& feature : m_features | std::views::values)
	{
		if (feature->IsEnabled())
		{
			feature->OnEndFrame();
		}
	}

	ClearDescriptors();
	ABaseRenderer::EndFrame();
}

void Rendering::Core::CompositeRenderer::DrawEntity(
	Rendering::Data::PipelineState p_pso,
	const Entities::Drawable& p_drawable
)
{
	ZoneScoped;

	// Ensure the drawable is valid.
	// If not, skip the draw call and the attached features.
	if (!IsDrawable(p_drawable))
	{
		return;
	}

	for (const auto& feature : m_features | std::views::values)
	{
		if (feature->IsEnabledFor(typeid(m_currentPass.value())))
		{
			feature->OnBeforeDraw(p_pso, p_drawable);
		}
	}

	ABaseRenderer::DrawEntity(p_pso, p_drawable);

	for (const auto& feature : m_features | std::views::values)
	{
		if (feature->IsEnabledFor(typeid(m_currentPass.value())))
		{
			feature->OnAfterDraw(p_drawable);
		}
	}
}
