

#pragma once

#include <Maths/FVector2.h>

#include "UI/Widgets/AWidget.h"

namespace UI::Widgets::Layout
{

	class Dummy : public AWidget
	{
	public:

		Dummy(const Maths::FVector2& p_size = { 0.0f, 0.0f });

	protected:
		void _Draw_Impl() override;

	public:
		Maths::FVector2 size;
	};
}