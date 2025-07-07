#pragma once
#include "UI/Widgets/InputFields/InputSingleScalar.h"
namespace UI::Widgets::InputFields
{

	class InputDouble : public InputSingleScalar<double>
	{
	public:

		InputDouble
		(
			double p_defaultValue = 0.0,
			double p_step = 0.1,
			double p_fastStep = 0.0,
			const std::string& p_label = "",
			const std::string& p_format = "%.6f",
			bool p_selectAllOnClick = true
		);
	};
}