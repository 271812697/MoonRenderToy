#pragma once
#include <UI/Panels/PanelWindow.h>
#include <UI/Widgets/Layout/GroupCollapsable.h>
#include <UI/Widgets/InputFields/InputText.h>
#include <UI/Widgets/Visual/Separator.h>
#include <UI/Widgets/Selection/CheckBox.h>
#include <UI/Widgets/Buttons/Button.h>
#include <UI/Widgets/Selection/ComboBox.h>
namespace UI::Panels {
	class Inspector : public UI::Panels::PanelWindow
	{
	public:

		Inspector
		(
			const std::string& p_title,
			bool p_opened,
			const UI::Settings::PanelWindowSettings& p_windowSettings
		);
		void init();

		~Inspector();

		virtual void InstallUI() {
		

		}





	protected:
		UI::Widgets::Layout::Group* m_inspectorHeader;
	};
}