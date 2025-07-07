

#pragma once

#include "UI/Widgets/AWidget.h"

namespace UI::Widgets::Layout
{
	/**
	* Widget that adds an empty line to the panel
	*/
	class NewLine : public AWidget
	{
	protected:
		void _Draw_Impl() override;
	};
}