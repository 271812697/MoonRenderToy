

#pragma once

#include <string>

#include <Maths/FVector2.h>

#include "UI/Types/Color.h"
#include "UI/Widgets/Buttons/AButton.h"

namespace UI::Widgets::Buttons
{

	class ButtonColored : public AButton
	{
	public:

		ButtonColored(const std::string& p_label = "", const Types::Color& p_color = {}, const Maths::FVector2& p_size =Maths::FVector2(0.f, 0.f), bool p_enableAlpha = true);

	protected:
		void _Draw_Impl() override;

	public:
		std::string label;
		Types::Color color;
		Maths::FVector2 size;
		bool enableAlpha;
	};
}