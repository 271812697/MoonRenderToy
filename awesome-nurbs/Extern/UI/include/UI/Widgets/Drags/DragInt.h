#pragma once
#include "UI/Widgets/Drags/DragSingleScalar.h"
namespace UI::Widgets::Drags
{

	class DragInt : public DragSingleScalar<int>
	{
	public:

		DragInt
		(
			int p_min = 0,
			int p_max = 100,
			int p_value = 50,
			float p_speed = 1.0f,
			const std::string& p_label = "",
			const std::string& p_format = "%d"
		);
	};
}