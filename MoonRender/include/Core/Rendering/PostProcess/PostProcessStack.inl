#pragma once
#include "Core/Rendering/PostProcess/PostProcessStack.h"

namespace Core::Rendering::PostProcess
{
	template <typename Effect, typename Settings>
	inline void PostProcessStack::Set(const Settings& p_settings)
	{
		static_assert(std::is_base_of<EffectSettings, Settings>::value,
			"Settings must derive from PostProcessSettings");
		m_settings[typeid(Effect)] = std::make_unique<Settings>(p_settings);
	}

	template <typename Effect, typename Settings>
	Settings& PostProcessStack::Get()
	{
		static_assert(std::is_base_of<EffectSettings, Settings>::value,
			"Settings must derive from PostProcessSettings");
		auto it = m_settings.find(typeid(Effect));
	
		return static_cast<Settings&>(*it->second);
	}

	template <typename Effect, typename Settings>
	const Settings& PostProcessStack::Get() const
	{
		static_assert(std::is_base_of<EffectSettings, Settings>::value,
			"Settings must derive from PostProcessSettings");
		auto it = m_settings.find(typeid(Effect));
		
		return static_cast<const Settings&>(*it->second);
	}
}
