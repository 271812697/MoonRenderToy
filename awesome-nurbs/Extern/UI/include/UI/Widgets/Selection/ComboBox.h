

#pragma once

#include <map>

#include "tools/Event.h"

#include "UI/Widgets/DataWidget.h"

namespace UI::Widgets::Selection
{

	class ComboBox : public DataWidget<int>
	{
	public:


		ComboBox(int p_currentChoice = 0);

	protected:
		void _Draw_Impl() override;

	public:
		std::map<int, std::string> choices;
		int currentChoice;

	public:
		Tools::Eventing::Event<int> ValueChangedEvent;
	};
}