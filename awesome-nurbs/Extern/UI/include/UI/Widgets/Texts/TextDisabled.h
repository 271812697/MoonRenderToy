#pragma once
#include "UI/Widgets/Texts/Text.h"
namespace UI::Widgets::Texts
{

	class TextDisabled : public Text
	{
	public:

		TextDisabled(const std::string& p_content = "");

	protected:
		virtual void _Draw_Impl() override;
	};
}