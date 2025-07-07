#pragma once
#include "UI/Widgets/Drags/DragSingleScalar.h"

namespace UI::Widgets::Drags
{

	class DragFloat : public DragSingleScalar<float>
	{
	public:

		DragFloat
		(
			float p_min = 0.0f,
			float p_max = 1.0f,
			float p_value = 0.5f,
			float p_speed = 0.1f,
			const std::string& p_label = "",
			const std::string& p_format = "%.3f"
		);
	};
}