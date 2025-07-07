

#pragma once

#include <Maths/FVector2.h>

#include "UI/Widgets/AWidget.h"

namespace UI::Widgets::Visual
{

	class ProgressBar : public AWidget
	{
	public:

		ProgressBar(float p_fraction = 0.0f, const Maths::FVector2& p_size = { 0.0f, 0.0f }, const std::string& p_overlay = "");

	protected:
		void _Draw_Impl() override;

	public:
		float fraction;
		Maths::FVector2 size;
		std::string overlay;
	};
}