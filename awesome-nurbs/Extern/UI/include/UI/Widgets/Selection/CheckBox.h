

#pragma once

#include <tools/Event.h>

#include "UI/Widgets/DataWidget.h"

namespace UI::Widgets::Selection
{

	class CheckBox : public DataWidget<bool>
	{
	public:

		CheckBox(bool p_value = false, const std::string& p_label = "");

	protected:
		void _Draw_Impl() override;

	public:
		bool value;
		std::string label;
		Tools::Eventing::Event<bool> ValueChangedEvent;
	};
}