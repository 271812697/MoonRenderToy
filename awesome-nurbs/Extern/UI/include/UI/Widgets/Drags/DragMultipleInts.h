#pragma once
#include "UI/Widgets/Drags/DragMultipleScalars.h"
namespace UI::Widgets::Drags
{

	template <size_t _Size>
	class DragMultipleInts : public DragMultipleScalars<int, _Size>
	{
	public:

		DragMultipleInts
		(
			int p_min = 0,
			int p_max = 100,
			int p_value = 50,
			float p_speed = 1.0f,
			const std::string& p_label = "",
			const std::string& p_format = "%d"
		) : DragMultipleScalars<int, _Size>(ImGuiDataType_::ImGuiDataType_S32, p_min, p_max, p_value, p_speed, p_label, p_format) {}
	};
}