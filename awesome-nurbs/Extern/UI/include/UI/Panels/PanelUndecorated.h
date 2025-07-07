#pragma once
#include "UI/Panels/APanelTransformable.h"
namespace UI::Panels
{

	class PanelUndecorated : public APanelTransformable
	{
	public:
		void _Draw_Impl() override;

	private:
		int CollectFlags();
	};
}