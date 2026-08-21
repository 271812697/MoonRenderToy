#pragma once
#include "Qtimgui/imgui/imgui.h"
#include "implot.h"

namespace ImPlotCustom {
    void AddTextTransform(ImVec2 pos, float angle, ImU32 col, const char* text_begin, const char* text_end = nullptr);
	void ColormapScale(const char* label,double val, double scale_min, double scale_max, const ImVec2& pos= ImVec2(0, 0),const ImVec2& size = ImVec2(0, 0), const char* format = "%g", ImPlotColormapScaleFlags flags = 0, ImPlotColormap cmap = IMPLOT_AUTO);
}