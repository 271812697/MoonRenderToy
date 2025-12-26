#include "Core/Rendering/PostProcess/PostProcessStack.h"
#include "Core/Rendering/PostProcess/BloomEffect.h"
#include "Core/Rendering/PostProcess/FXAAEffect.h"
#include "Core/Rendering/PostProcess/TonemappingEffect.h"
#include "Core/Rendering/PostProcess/AutoExposureEffect.h"
#include <assert.h>
Core::Rendering::PostProcess::PostProcessStack::PostProcessStack()
{
	Set<BloomEffect>(BloomSettings());
	Set<AutoExposureEffect>(AutoExposureSettings());
	Set<FXAAEffect>(FXAASettings());
	Set<TonemappingEffect>(TonemappingSettings());
}

Core::Rendering::PostProcess::EffectSettings& Core::Rendering::PostProcess::PostProcessStack::Get(const std::type_index& type)
{
	auto it = m_settings.find(type);
	assert(it != m_settings.end()&& "Settings not found for the given Effect type");
	return *it->second;
}

const Core::Rendering::PostProcess::EffectSettings& Core::Rendering::PostProcess::PostProcessStack::Get(const std::type_index& type) const
{
	auto it = m_settings.find(type);
	assert(it != m_settings.end()&&"Settings not found for the given Effect type");
	return *it->second;
}