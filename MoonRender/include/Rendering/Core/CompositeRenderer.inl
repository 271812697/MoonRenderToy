#pragma once
#include <ranges>
#include <Rendering/Core/CompositeRenderer.h>

namespace Rendering::Core
{
	template<Types::RenderFeatureType T, Features::EFeatureExecutionPolicy Policy, typename... Args>
	T& CompositeRenderer::AddFeature(Args&&... args)
	{

		T* feature = new T(*this, Policy, std::forward<Args>(args)...);
		m_features.emplace(typeid(T), feature);
		return *feature;
	}

	template<Types::RenderFeatureType T>
	inline bool CompositeRenderer::RemoveFeature()
	{
		return m_features.erase(typeid(T)) > 0;
	}

	template<Types::RenderFeatureType T>
	inline T& CompositeRenderer::GetFeature() const
	{
		auto it = m_features.find(typeid(T));
		return *dynamic_cast<T*>(it->second.get());
	}

	template<Types::RenderFeatureType T>
	inline bool CompositeRenderer::HasFeature() const
	{
		auto it = m_features.find(typeid(T));
		return it != m_features.end();
	}

	template<Types::RenderPassType T, typename ...Args>
	inline T& CompositeRenderer::AddPass(const std::string& p_name, uint32_t p_order, Args&& ...p_args)
	{
		

		T* pass = new T(*this, std::forward<Args>(p_args)...);
		m_passes.emplace(p_order, std::make_pair(p_name, std::unique_ptr<ARenderPass>(pass)));
		return *pass;
	}

	template<Types::RenderPassType T>
	inline T& CompositeRenderer::GetPass(const std::string& p_name) const
	{
		for (const auto& pass : m_passes | std::views::values)
		{
			if (pass.first == p_name)
			{
				return dynamic_cast<T&>(*pass.second.get());
			}
		}

	
		return *static_cast<T*>(nullptr);
	}
}
