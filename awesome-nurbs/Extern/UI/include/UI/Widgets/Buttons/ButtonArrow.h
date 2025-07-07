
#pragma once

#include <string>

#include "UI/Widgets/Buttons/AButton.h"

namespace UI::Widgets::Buttons
{

	class ButtonArrow : public AButton
	{
	public:

		ButtonArrow(ImGuiDir p_direction = ImGuiDir_None);

	protected:
		void _Draw_Impl() override;

	public:
		ImGuiDir direction;
	};
}