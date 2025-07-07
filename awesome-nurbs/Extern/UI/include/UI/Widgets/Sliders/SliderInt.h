
#pragma once

#include "UI/Widgets/Sliders/SliderSingleScalar.h"

namespace UI::Widgets::Sliders
{

	class SliderInt : public SliderSingleScalar<int>
	{
	public:

		SliderInt
		(
			int p_min = 0,
			int p_max = 100,
			int p_value = 50,
			ESliderOrientation p_orientation = ESliderOrientation::HORIZONTAL,
			const std::string& p_label = "",
			const std::string& p_format = "%d"
		);
	};
}