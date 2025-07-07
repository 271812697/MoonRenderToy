

#pragma once

#include "tools/Event.h"

#include "UI/Widgets/DataWidget.h"

namespace UI::Widgets::Selection
{
	class RadioButtonLinker;


	class RadioButton : public DataWidget<bool>
	{
		friend RadioButtonLinker;
		
	public:

		RadioButton(bool p_selected = false, const std::string& p_label = "");


		void Select();


		bool IsSelected() const;


		bool GetRadioID() const;

	protected:
		void _Draw_Impl() override;

	public:
		std::string label;
		Tools::Eventing::Event<int> ClickedEvent;

	private:
		bool m_selected = false;
		int m_radioID = 0;
	};
}