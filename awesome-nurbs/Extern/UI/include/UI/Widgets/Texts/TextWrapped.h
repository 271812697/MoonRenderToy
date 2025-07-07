#pragma once
#include "UI/Widgets/Texts/Text.h"
namespace UI::Widgets::Texts
{

	class TextWrapped : public Text
	{
	public:

		TextWrapped(const std::string& p_content = "");

	protected:
		virtual void _Draw_Impl() override;
	};
}