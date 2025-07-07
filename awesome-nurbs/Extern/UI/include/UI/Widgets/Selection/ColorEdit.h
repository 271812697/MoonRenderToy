

#pragma once

#include "tools/Event.h"

#include "UI/Widgets/DataWidget.h"
#include "UI/Types/Color.h"

namespace UI::Widgets::Selection
{

	class ColorEdit : public DataWidget<Types::Color>
	{
	public:

		ColorEdit(bool p_enableAlpha = false, const Types::Color& p_defaultColor = {});

	protected:
		void _Draw_Impl() override;

	public:
		bool enableAlpha;
		Types::Color color;
		Tools::Eventing::Event<Types::Color&> ColorChangedEvent;
	};
}