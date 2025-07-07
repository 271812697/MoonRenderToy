#pragma once
#include <vector>
#include <memory>
#include <algorithm>
#include "tools/Event.h"
#include "UI/Panels/APanel.h"
#include "UI/Widgets/Menu/MenuList.h"
namespace UI::Panels
{

	class PanelMenuBar : public APanel
	{
	protected:
		void _Draw_Impl() override;
	};
}