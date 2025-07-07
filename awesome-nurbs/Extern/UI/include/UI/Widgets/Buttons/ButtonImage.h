

#pragma once

#include <Maths/FVector2.h>

#include "UI/Internal/TextureID.h"
#include "UI/Types/Color.h"
#include "UI/Widgets/Buttons/AButton.h"

namespace UI::Widgets::Buttons
{

	class ButtonImage : public AButton
	{
	public:

		ButtonImage(uint32_t p_textureID, const Maths::FVector2& p_size);

	protected:
		void _Draw_Impl() override;

	public:
		bool disabled = false;

		UI::Types::Color background = { 0, 0, 0, 0 };
		UI::Types::Color tint = { 1, 1, 1, 1 };

		Internal::TextureID textureID;
		Maths::FVector2 size;
	};
}