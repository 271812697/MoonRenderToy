
#pragma once

#include "tools/Event.h"

#include "UI/Widgets/Texts/Text.h"
namespace UI::Widgets::Texts
{

	class TextSelectable : public Text
	{
	public:

		TextSelectable(const std::string& p_content = "", bool p_selected = false, bool p_disabled = false);

	protected:
		virtual void _Draw_Impl() override;

	public:
		bool selected;
		bool disabled;

		Tools::Eventing::Event<bool> ClickedEvent;
		Tools::Eventing::Event<> SelectedEvent;
		Tools::Eventing::Event<> UnselectedEvent;
	};
}