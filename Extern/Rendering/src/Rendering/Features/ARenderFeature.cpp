#include "Rendering/Features/ARenderFeature.h"
#include "Rendering/Core/ABaseRenderer.h"
#include "Rendering/Core/CompositeRenderer.h"
#include <assert.h>

Rendering::Features::ARenderFeature::ARenderFeature(
	Core::CompositeRenderer& p_renderer,
	EFeatureExecutionPolicy p_executionPolicy
) :
	m_renderer(p_renderer),
	m_executionPolicy(p_executionPolicy)
{
}

bool Rendering::Features::ARenderFeature::IsEnabled() const
{
	return m_executionPolicy != EFeatureExecutionPolicy::NEVER;
}

bool Rendering::Features::ARenderFeature::IsEnabledFor(std::type_index p_type) const
{
	return (m_executionPolicy == EFeatureExecutionPolicy::ALWAYS) ||
		(m_executionPolicy == EFeatureExecutionPolicy::WHITELIST_ONLY && m_whitelist.contains(p_type)) ||
		(m_executionPolicy == EFeatureExecutionPolicy::DEFAULT && !m_blacklist.contains(p_type));
}

Rendering::Features::ARenderFeature& Rendering::Features::ARenderFeature::Except(std::type_index p_type)
{
	assert(!m_renderer.IsDrawing()&& "Cannot add a render pass to the blacklist while rendering is in progress.");
	m_blacklist.insert(p_type);
	return *this;
}

Rendering::Features::ARenderFeature& Rendering::Features::ARenderFeature::Include(std::type_index p_type)
{
	assert(!m_renderer.IsDrawing()&&"Cannot add a render pass to the whitelist while rendering is in progress.");
	m_whitelist.insert(p_type);
	return *this;
}

void Rendering::Features::ARenderFeature::SetExecutionPolicy(EFeatureExecutionPolicy p_policy)
{
	assert(!m_renderer.IsDrawing()&&"Cannot set the execution policy while rendering is in progress.");
	m_executionPolicy = p_policy;
}

Rendering::Features::EFeatureExecutionPolicy Rendering::Features::ARenderFeature::GetExecutionPolicy() const
{
	return m_executionPolicy;
}

void Rendering::Features::ARenderFeature::OnBeginFrame(const Data::FrameDescriptor& p_frameDescriptor)
{
}

void Rendering::Features::ARenderFeature::OnEndFrame()
{
}

void Rendering::Features::ARenderFeature::OnBeforeDraw(Data::PipelineState& p_pso, const Entities::Drawable& p_drawable)
{
}

void Rendering::Features::ARenderFeature::OnAfterDraw(const Entities::Drawable& p_drawable)
{
}
