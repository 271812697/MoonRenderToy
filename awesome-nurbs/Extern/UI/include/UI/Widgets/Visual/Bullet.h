
#pragma once

#include "UI/Widgets/AWidget.h"

namespace UI::Widgets::Visual
{

	class Bullet : public AWidget
	{
	protected:
		virtual void _Draw_Impl() override;
	};
}