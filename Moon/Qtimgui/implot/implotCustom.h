#pragma once
#include "Qtimgui/imgui/imgui.h"
#include "implot.h"

namespace ImPlotCustom {
	void ColormapScale(const char* label,double val, double scale_min, double scale_max, const ImVec2& pos= ImVec2(0, 0),const ImVec2& size = ImVec2(0, 0), const char* format = "%g", ImPlotColormapScaleFlags flags = 0, ImPlotColormap cmap = IMPLOT_AUTO);
}