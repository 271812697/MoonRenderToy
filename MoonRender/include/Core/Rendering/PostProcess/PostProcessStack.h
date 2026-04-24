#pragma once
#include <Core/Rendering/PostProcess/AEffect.h>

namespace Core::Rendering::PostProcess
{
	class PostProcessStack
	{
	public:
		PostProcessStack();
		template <typename Effect, typename Settings>
		void Set(const Settings& p_settings);
		template <typename Effect, typename Settings>
		Settings& Get();
		template <typename Effect, typename Settings>
		const Settings& Get() const;
		EffectSettings& Get(const std::type_index& p_type);
		const EffectSettings& Get(const std::type_index& p_type) const;

	private:
		std::unordered_map<std::type_index, std::unique_ptr<EffectSettings>> m_settings;
	};
}

#include <Core/Rendering/PostProcess/PostProcessStack.inl>
