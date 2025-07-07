

#pragma once

#include <vector>

#include <tools/Event.h>

#include "UI/Widgets/Layout/Group.h"

namespace UI::Widgets::Menu
{

	class MenuList : public Layout::Group
	{
	public:

		MenuList(const std::string& p_name, bool p_locked = false);

	protected:
		virtual void _Draw_Impl() override;

	public:
		std::string name;
		bool locked;
		Tools::Eventing::Event<> ClickedEvent;

	private:
		bool m_opened;
	};
}