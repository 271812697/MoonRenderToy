

#pragma once

#include "UI/Widgets/Selection/RadioButton.h"

namespace UI::Widgets::Selection
{

	class RadioButtonLinker : public DataWidget<int>
	{
	public:

		RadioButtonLinker();


		void Link(RadioButton& p_radioButton);


		void Unlink(RadioButton& p_radioButton);

	
		int GetSelected() const;

	protected:
		void _Draw_Impl() override;

	private:
		void OnRadioButtonClicked(int p_radioID);

	public:
		Tools::Eventing::Event<int> ValueChangedEvent;

	private:
		int m_availableRadioID = 0;
		int m_selected = -1;
		std::vector<std::pair<Tools::Eventing::ListenerID, std::reference_wrapper<RadioButton>>> m_radioButtons;
	};
}