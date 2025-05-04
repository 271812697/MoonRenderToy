/**
* @project: erload
* @author: erload Tech.
* @licence: MIT
*/

#include "Core/Rendering/PostProcess/AEffect.h"

Core::Rendering::PostProcess::AEffect::AEffect(::Rendering::Core::CompositeRenderer& p_renderer) :
	m_renderer(p_renderer)
{
}

bool Core::Rendering::PostProcess::AEffect::IsApplicable(const EffectSettings& p_settings) const
{
	return p_settings.enabled;
}
