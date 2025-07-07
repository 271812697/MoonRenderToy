
#pragma once

#include "UI/Plugins/IPlugin.h"
#include "UI/Internal/WidgetContainer.h"
#include "UI/Widgets/Menu/MenuList.h"
#include "UI/Widgets/Menu/MenuItem.h"

namespace UI::Plugins
{

	class ContextualMenu : public IPlugin, public Internal::WidgetContainer
	{
	public:

		void Execute();

		void Close();
	};
}
