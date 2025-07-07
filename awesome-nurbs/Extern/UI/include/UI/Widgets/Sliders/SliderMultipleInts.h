#pragma once
#include "UI/Widgets/Sliders/SliderMultipleScalars.h"
namespace UI::Widgets::Sliders
{

	template <size_t _Size>
	class SliderMultipleInts : public SliderMultipleScalars<int, _Size>
	{
	public:

		SliderMultipleInts
		(
			int p_min = 0,
			int p_max = 100,
			int p_value = 50,
			const std::string& p_label = "",
			const std::string& p_format = "%d"
		) : SliderMultipleScalars<int, _Size>(ImGuiDataType_::ImGuiDataType_S32, p_min, p_max, p_value, p_label, p_format) {}
	};
}