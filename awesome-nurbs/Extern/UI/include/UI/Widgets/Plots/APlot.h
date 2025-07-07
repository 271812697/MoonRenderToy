

#pragma once

#include <vector>
#include <limits>

#include <Maths/FVector2.h>

#include "UI/Widgets/AWidget.h"
#include "UI/Widgets/DataWidget.h"

namespace UI::Widgets::Plots
{

	class APlot : public DataWidget<std::vector<float>>
	{
	public:

		APlot
		(
			const std::vector<float>& p_data = std::vector<float>(),
			float p_minScale = std::numeric_limits<float>::min(),
			float p_maxScale = std::numeric_limits<float>::max(),
			const Maths::FVector2& p_size = { 0.0f, 0.0f },
			const std::string& p_overlay = "",
			const std::string& p_label = "",
			int p_forceHover = -1
		);

	protected:
		virtual void _Draw_Impl() override = 0;

	public:
		std::vector<float> data;
		float minScale;
		float maxScale;
		Maths::FVector2 size;
		std::string overlay;
		std::string label;
		int forceHover;
	};
}