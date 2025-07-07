
#pragma once

#include "UI/Widgets/InputFields/InputMultipleScalars.h"

namespace UI::Widgets::InputFields
{

	template <size_t _Size>
	class InputMultipleDoubles : public InputMultipleScalars<double, _Size>
	{
	public:

		InputMultipleDoubles
		(
			double p_defaultValue = 0.0,
			double p_step = 0.1,
			double p_fastStep = 0.0,
			const std::string& p_label = "",
			const std::string& p_format = "%.6f",
			bool p_selectAllOnClick = true
		) : InputMultipleScalars<double, _Size>(ImGuiDataType_::ImGuiDataType_Double, p_defaultValue, p_step, p_fastStep, p_label, p_format, p_selectAllOnClick) {}
	};
}