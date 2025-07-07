

#pragma once

#include "UI/Widgets/Texts/Text.h"

namespace UI::Widgets::Texts
{

	class TextLabelled : public Text
	{
	public:

		TextLabelled(const std::string& p_content = "", const std::string& p_label = "");

	protected:
		virtual void _Draw_Impl() override;

	public:
		std::string label;
	};
}