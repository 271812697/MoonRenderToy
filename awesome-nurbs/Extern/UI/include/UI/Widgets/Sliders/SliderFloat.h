
#pragma once

#include "UI/Widgets/Sliders/SliderSingleScalar.h"

namespace UI::Widgets::Sliders
{

	class SliderFloat : public SliderSingleScalar<float>
	{
	public:

		SliderFloat
		(
			float p_min = 0.0f,
			float p_max = 1.0f,
			float p_value = 0.5f,
			ESliderOrientation p_orientation = ESliderOrientation::HORIZONTAL,
			const std::string& p_label = "",
			const std::string& p_format = "%.3f"
		);
	};
}