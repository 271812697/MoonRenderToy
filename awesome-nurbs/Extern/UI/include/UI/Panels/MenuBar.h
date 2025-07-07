#pragma once
#include <UI/Panels/PanelMenuBar.h>
#include <UI/Panels/PanelWindow.h>
#include <UI/Widgets/Menu/MenuItem.h>
namespace UI::Panels {
	class MenuBar : public UI::Panels::PanelMenuBar
	{
		using PanelMap = std::unordered_map<std::string, std::pair<std::reference_wrapper<UI::Panels::PanelWindow>, std::reference_wrapper<UI::Widgets::Menu::MenuItem>>>;

	public:

		MenuBar();



		void RegisterPanel(const std::string& p_name, UI::Panels::PanelWindow& p_panel);

	private:
		void CreateFileMenu();

		void CreateResourcesMenu();
		void CreateWindowMenu();

		void UpdateToggleableItems();
		void OpenEveryWindows(bool p_state);

	private:
		PanelMap m_panels;
		UI::Widgets::Menu::MenuList* m_windowMenu = nullptr;
	};

}