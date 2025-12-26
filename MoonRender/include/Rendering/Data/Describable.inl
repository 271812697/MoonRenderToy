#pragma once
#include <Rendering/Data/Describable.h>

namespace Rendering::Data
{
	template<typename T>
	inline void Describable::AddDescriptor(T&& p_descriptor)
	{
		
		m_descriptors.emplace(typeid(T), std::move(p_descriptor));
	}

	template<typename T>
	inline void Describable::RemoveDescriptor()
	{
		if (auto it = m_descriptors.find(typeid(T)); it != m_descriptors.end())
		{
			m_descriptors.erase(it);
		}
	}

	template<typename T>
	inline bool Describable::HasDescriptor() const
	{
		return m_descriptors.contains(typeid(T));
	}

	template<typename T>
	inline const T& Describable::GetDescriptor() const
	{
		auto it = m_descriptors.find(typeid(T));
		
		return std::any_cast<const T&>(it->second);
	}

	template<typename T>
	inline bool Describable::TryGetDescriptor(Tools::Utils::OptRef<const T>& p_outDescriptor) const
	{
		if (auto it = m_descriptors.find(typeid(T)); it != m_descriptors.end())
		{
			p_outDescriptor = std::any_cast<const T&>(it->second);
			return true;
		}

		return false;
	}
}
