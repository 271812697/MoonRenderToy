
#pragma once

#include "tools/Event.h"

#include "UI/Widgets/Texts/Text.h"

namespace UI::Widgets::Texts
{

	class TextClickable : public Text
	{
	public:

		TextClickable(const std::string& p_content = "");

	protected:
		virtual void _Draw_Impl() override;

	public:
		Tools::Eventing::Event<> ClickedEvent;
		Tools::Eventing::Event<> DoubleClickedEvent;
	};
}