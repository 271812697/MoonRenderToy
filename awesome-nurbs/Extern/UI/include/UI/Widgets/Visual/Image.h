#pragma once
#include <Maths/FVector2.h>
#include "UI/Internal/TextureID.h"
#include "UI/Widgets/AWidget.h"
namespace UI::Widgets::Visual
{

	class Image : public AWidget
	{
	public:

		Image(uint32_t p_textureID, const Maths::FVector2& p_size);

	protected:
		void _Draw_Impl() override;

	public:
		Internal::TextureID textureID;
		Maths::FVector2 size;
	};
}