

#pragma once

#include <vector>

#include "UI/Plugins/IPlugin.h"

namespace UI::Plugins
{

	class Pluginable
	{
	public:

		~Pluginable()
		{
			RemoveAllPlugins();
		}


		template<typename T, typename... Args>
		T& AddPlugin(Args&& ... p_args)
		{
			static_assert(std::is_base_of<Plugins::IPlugin, T>::value, "T should derive from IPlugin");

			T* newPlugin = new T(std::forward<Args>(p_args)...);
			m_plugins.push_back(newPlugin);
			return *newPlugin;
		}

		template<typename T>
		T* GetPlugin()
		{
			static_assert(std::is_base_of<Plugins::IPlugin, T>::value, "T should derive from IPlugin");

			for (auto it = m_plugins.begin(); it != m_plugins.end(); ++it)
			{
				T* result = dynamic_cast<T*>(*it);
				if (result)
					return result;
			}

			return nullptr;
		}


		void ExecutePlugins()
		{
			for (auto& plugin : m_plugins)
				plugin->Execute();
		}

	
		void RemoveAllPlugins()
		{
			for (auto& plugin : m_plugins)
				delete plugin;

			m_plugins.clear();
		}

	private:
		std::vector<IPlugin*> m_plugins;
	};
}