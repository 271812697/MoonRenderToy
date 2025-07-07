
#pragma once

#include "UI/Widgets/Drags/DragSingleScalar.h"

namespace UI::Widgets::Drags
{

	class DragDouble : public DragSingleScalar<double>
	{
	public:

		DragDouble
		(
			double p_min = 0.0,
			double p_max = 1.0,
			double p_value = 0.5,
			float p_speed = 0.1f,
			const std::string& p_label = "",
			const std::string& p_format = "%.6f"
		);
	};
}