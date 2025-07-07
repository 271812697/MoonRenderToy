#pragma once
#include "UI/Widgets/Sliders/SliderMultipleScalars.h"
namespace UI::Widgets::Sliders
{

	template <size_t _Size>
	class SliderMultipleFloats : public SliderMultipleScalars<float, _Size>
	{
	public:

		SliderMultipleFloats
		(
			float p_min = 0.0f,
			float p_max = 1.0f,
			float p_value = 0.5f,
			const std::string& p_label = "",
			const std::string& p_format = "%.3f"
		) : SliderMultipleScalars<float, _Size>(ImGuiDataType_::ImGuiDataType_Float, p_min, p_max, p_value, p_label, p_format) {}
	};
}