#pragma once
#include "UI/Widgets/Drags/DragMultipleScalars.h"
namespace UI::Widgets::Drags
{

	template <size_t _Size>
	class DragMultipleDoubles : public DragMultipleScalars<double, _Size>
	{
	public:

		DragMultipleDoubles
		(
			double p_min = 0.0,
			double p_max = 1.0,
			double p_value = 0.5,
			float p_speed = 0.1f,
			const std::string& p_label = "",
			const std::string& p_format = "%.6f"
		) : DragMultipleScalars<double, _Size>(ImGuiDataType_::ImGuiDataType_Double, p_min, p_max, p_value, p_speed, p_label, p_format) {}
	};
}