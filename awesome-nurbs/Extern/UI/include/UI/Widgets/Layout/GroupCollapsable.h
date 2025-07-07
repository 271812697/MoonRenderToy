#pragma once
#include <vector>
#include <tools/Event.h>

#include "UI/Widgets/Layout/Group.h"

namespace UI::Widgets::Layout
{

	class GroupCollapsable : public Group
	{
	public:

		GroupCollapsable(const std::string& p_name = "");

	protected:
		virtual void _Draw_Impl() override;

	public:
		std::string name;
		bool closable = false;
		bool opened = true;
		Tools::Eventing::Event<> CloseEvent;
		Tools::Eventing::Event<> OpenEvent;
	};
}