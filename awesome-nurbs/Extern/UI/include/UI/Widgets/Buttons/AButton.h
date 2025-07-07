
#pragma once

#include <string>

#include "tools/Event.h"

#include "UI/Widgets/AWidget.h"

namespace UI::Widgets::Buttons
{
	/**
	* Base class for any button widget
	*/
	class AButton : public AWidget
	{
	protected:
		void _Draw_Impl() override = 0;

	public:
		Tools::Eventing::Event<> ClickedEvent;
	};
}