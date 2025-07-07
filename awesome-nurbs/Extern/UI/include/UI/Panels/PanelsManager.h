#pragma once
#include <unordered_map>
#include "MenuBar.h"
#include "UI/Panels/APanel.h"
#include "UI/Panels/PanelWindow.h"
#include "UI/Modules/Canvas.h"
namespace UI::Panels {
	class PanelsManager
	{
	public:

		PanelsManager(UI::Modules::Canvas& p_canvas);


		template<typename T, typename... Args>
		void CreatePanel(const std::string& p_id, Args&&... p_args)
		{
			if constexpr (std::is_base_of<UI::Panels::PanelWindow, T>::value)
			{
				m_panels.emplace(p_id, std::make_unique<T>(p_id, std::forward<Args>(p_args)...));
				T& instance = *static_cast<T*>(m_panels.at(p_id).get());
				GetPanelAs<MenuBar>("Menu Bar").RegisterPanel(instance.name, instance);
			}
			else
			{
				m_panels.emplace(p_id, std::make_unique<T>(std::forward<Args>(p_args)...));
			}

			m_canvas.AddPanel(*m_panels.at(p_id));
		}

		template<typename T>
		T& GetPanelAs(const std::string& p_id)
		{
			return *static_cast<T*>(m_panels[p_id].get());
		}

	private:
		std::unordered_map<std::string, std::unique_ptr<UI::Panels::APanel>> m_panels;
		UI::Modules::Canvas& m_canvas;
	};

}