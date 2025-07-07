
#pragma once

#include "UI/Widgets/AWidget.h"

namespace UI::Widgets::Visual
{

	class Separator : public AWidget
	{
	protected:
		void _Draw_Impl() override;
	};
}