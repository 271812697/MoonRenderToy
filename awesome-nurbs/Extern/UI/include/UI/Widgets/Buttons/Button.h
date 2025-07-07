
#pragma once

#include <string>

#include <Maths/FVector2.h>

#include "UI/Widgets/Buttons/AButton.h"
#include "UI/Types/Color.h"

namespace UI::Widgets::Buttons
{
	class Button : public AButton
	{
	public:
		Button(const std::string& p_label = "", const Maths::FVector2& p_size = Maths::FVector2(0.f, 0.f), bool p_disabled = false);
	protected:
		void _Draw_Impl() override;
	public:
		std::string label;
		Maths::FVector2 size;
		bool disabled = false;
		Types::Color idleBackgroundColor;
		Types::Color hoveredBackgroundColor;
		Types::Color clickedBackgroundColor;
		Types::Color textColor;
	};
}