

#pragma once

#include "UI/Widgets/InputFields/InputMultipleScalars.h"

namespace UI::Widgets::InputFields
{
	namespace InputFields {
		template <size_t _Size>
		class InputMultipleFloats : public InputMultipleScalars<float, _Size>
		{
		public:

			InputMultipleFloats
			(
				float p_defaultValue = 0.0f,
				float p_step = 0.1f,
				float p_fastStep = 0.0f,
				const std::string& p_label = "",
				const std::string& p_format = "%.3f",
				bool p_selectAllOnClick = true
			) : InputMultipleScalars<float, _Size>(ImGuiDataType_::ImGuiDataType_Float, p_defaultValue, p_step, p_fastStep, p_label, p_format, p_selectAllOnClick) {}
		};
	}

}