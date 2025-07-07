
#pragma once

#include <string>

#include "UI/Widgets/Buttons/AButton.h"
#include "UI/Types/Color.h"

namespace UI::Widgets::Buttons
{

	class ButtonSmall : public AButton
	{
	public:

		ButtonSmall(const std::string& p_label = "");

	protected:
		void _Draw_Impl() override;

	public:
		std::string label;

		Types::Color idleBackgroundColor;
		Types::Color hoveredBackgroundColor;
		Types::Color clickedBackgroundColor;

		Types::Color textColor;
	};
}