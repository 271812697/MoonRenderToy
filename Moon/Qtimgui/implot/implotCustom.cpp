#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif
#include "implotCustom.h"
#include "implot_internal.h"
using namespace ImPlot;
namespace ImPlotCustom {
	static void RenderArrowsForVerticalBar(ImDrawList* draw_list, ImVec2 pos, ImVec2 half_sz, float bar_w, float alpha)
	{
		ImU32 alpha8 = IM_F32_TO_INT8_SAT(alpha);
		ImGui::RenderArrowPointingAt(draw_list, ImVec2(pos.x + half_sz.x + 1, pos.y), ImVec2(half_sz.x + 2, half_sz.y + 1), ImGuiDir_Right, IM_COL32(0, 0, 0, alpha8));
		ImGui::RenderArrowPointingAt(draw_list, ImVec2(pos.x + half_sz.x, pos.y), half_sz, ImGuiDir_Right, IM_COL32(255, 255, 255, alpha8));
		ImGui::RenderArrowPointingAt(draw_list, ImVec2(pos.x + bar_w - half_sz.x - 1, pos.y), ImVec2(half_sz.x + 2, half_sz.y + 1), ImGuiDir_Left, IM_COL32(0, 0, 0, alpha8));
		ImGui::RenderArrowPointingAt(draw_list, ImVec2(pos.x + bar_w - half_sz.x, pos.y), half_sz, ImGuiDir_Left, IM_COL32(255, 255, 255, alpha8));
	}
	//y axis is down
	ImVec2 RotateVec2(ImVec2 pos, float cosVal, float sinVal) {
		return ImVec2(pos.x*cosVal+pos.y*sinVal,-pos.x*sinVal+pos.y*cosVal);
	}
	void AddTextTransform( ImVec2 pos, float angle, ImU32 col, const char* text_begin, const char* text_end)
	{
		ImDrawList* DrawList = ImGui::GetForegroundDrawList();
		if (!text_end)
			text_end = text_begin + strlen(text_begin);
		ImGuiContext& g = *GImGui;
#ifdef IMGUI_HAS_TEXTURES
		ImFontBaked* font = g.Font->GetFontBaked(g.FontSize);
		const float scale = g.FontSize / font->Size;
#else
		ImFont* font = g.Font;
		const float scale = g.FontSize / font->FontSize;
#endif
		// Align to be pixel perfect
		pos.x = ImFloor(pos.x);
		pos.y = ImFloor(pos.y);
		const char* s = text_begin;
		int chars_exp = (int)(text_end - s);
		int chars_rnd = 0;
		const int vtx_count_max = chars_exp * 4;
		const int idx_count_max = chars_exp * 6;
		DrawList->PrimReserve(idx_count_max, vtx_count_max);
		float cosVal = cosf(angle);
		float sinVal = sinf(angle);
		while (s < text_end) {
			unsigned int c = (unsigned int)*s;
			if (c < 0x80) {
				s += 1;
			}
			else {
				s += ImTextCharFromUtf8(&c, s, text_end);
				if (c == 0) // Malformed UTF-8?
					break;
			}
			const ImFontGlyph* glyph = font->FindGlyph((ImWchar)c);
			if (glyph == nullptr) {
				continue;
			}
			ImVec2 p0 = RotateVec2(ImVec2(glyph->X0, glyph->Y0), cosVal, sinVal);
			ImVec2 p1 = RotateVec2(ImVec2(glyph->X1, glyph->Y0), cosVal, sinVal);
			ImVec2 p2 = RotateVec2(ImVec2(glyph->X1, glyph->Y1), cosVal, sinVal);
			ImVec2 p3 = RotateVec2(ImVec2(glyph->X0, glyph->Y1), cosVal, sinVal);
			DrawList->PrimQuadUV(pos + p0 * scale, pos + p1* scale,
				pos + p2 * scale, pos + p3 * scale,
				ImVec2(glyph->U0, glyph->V0), ImVec2(glyph->U1, glyph->V0),
				ImVec2(glyph->U1, glyph->V1), ImVec2(glyph->U0, glyph->V1),
				col);
			pos.y -= glyph->AdvanceX * scale*sinVal;
			pos.x += glyph->AdvanceX * scale * cosVal;
			chars_rnd++;
		}
		// Give back unused vertices
		int chars_skp = chars_exp - chars_rnd;
		DrawList->PrimUnreserve(chars_skp * 6, chars_skp * 4);
		

	}
	void ColormapScale(const char* label, double val, double scale_min, double scale_max, const ImVec2& pos, const ImVec2& size, const char* format, ImPlotColormapScaleFlags flags, ImPlotColormap cmap)
	{
		ImGuiContext& G = *GImGui;

		ImVec2 label_size(0, 0);
		if (!ImHasFlag(flags, ImPlotColormapScaleFlags_NoLabel)) {
			label_size = ImGui::CalcTextSize(label, nullptr, true);
		}

		ImPlotContext& gp = *GImPlot;
		cmap = cmap == IMPLOT_AUTO ? gp.Style.Colormap : cmap;
		IM_ASSERT_USER_ERROR(cmap >= 0 && cmap < gp.ColormapData.Count, "Invalid colormap index!");

		ImVec2 frame_size = ImGui::CalcItemSize(size, 0, gp.Style.PlotDefaultSize.y);
		if (frame_size.y < gp.Style.PlotMinSize.y && size.y < 0.0f)
			frame_size.y = gp.Style.PlotMinSize.y;

		ImPlotRange range(ImMin(scale_min, scale_max), ImMax(scale_min, scale_max));
		gp.CTicker.Reset();
		Locator_Default(gp.CTicker, range, frame_size.y, true, Formatter_Default, (void*)format);

		const bool rend_label = label_size.x > 0;
		const float txt_off = gp.Style.LabelPadding.x;
		const float pad = txt_off + gp.CTicker.MaxSize.x + (rend_label ? txt_off + label_size.y : 0);
		float bar_w = 20;
		if (frame_size.x == 0)
			frame_size.x = bar_w + pad + 2 * gp.Style.PlotPadding.x;
		else {
			bar_w = frame_size.x - (pad + 2 * gp.Style.PlotPadding.x);
			if (bar_w < gp.Style.MajorTickLen.y)
				bar_w = gp.Style.MajorTickLen.y;
		}

		ImDrawList& DrawList = *ImGui::GetForegroundDrawList();
		ImRect bb_frame = ImRect(pos, pos + frame_size);



		const bool opposite = ImHasFlag(flags, ImPlotColormapScaleFlags_Opposite);
		const bool inverted = ImHasFlag(flags, ImPlotColormapScaleFlags_Invert);
		const bool reversed = scale_min > scale_max;

		float bb_grad_shift = opposite ? pad : 0;
		ImRect bb_grad(bb_frame.Min + gp.Style.PlotPadding + ImVec2(bb_grad_shift, 0),
			bb_frame.Min + ImVec2(bar_w + gp.Style.PlotPadding.x + bb_grad_shift,
				frame_size.y - gp.Style.PlotPadding.y));

		const ImU32 col_text = ImGui::GetColorU32(ImGuiCol_Text);
		const bool invert_scale = inverted ? (reversed ? false : true) : (reversed ? true : false);
		const float y_min = invert_scale ? bb_grad.Max.y : bb_grad.Min.y;
		const float y_max = invert_scale ? bb_grad.Min.y : bb_grad.Max.y;

		RenderColorBar(gp.ColormapData.GetKeys(cmap), gp.ColormapData.GetKeyCount(cmap), DrawList, bb_grad, true, !inverted, !gp.ColormapData.IsQual(cmap));
		for (int i = 0; i < gp.CTicker.TickCount(); ++i) {
			const double y_pos_plt = gp.CTicker.Ticks[i].PlotPos;
			const float y_pos = ImRemap((float)y_pos_plt, (float)range.Max, (float)range.Min, y_min, y_max);
			const float tick_width = gp.CTicker.Ticks[i].Major ? gp.Style.MajorTickLen.y : gp.Style.MinorTickLen.y;
			const float tick_thick = gp.CTicker.Ticks[i].Major ? gp.Style.MajorTickSize.y : gp.Style.MinorTickSize.y;
			const float tick_t = (float)((y_pos_plt - scale_min) / (scale_max - scale_min));
			const ImU32 tick_col = CalcTextColor(gp.ColormapData.LerpTable(cmap, tick_t));
			if (y_pos < bb_grad.Max.y - 2 && y_pos > bb_grad.Min.y + 2) {
				DrawList.AddLine(opposite ? ImVec2(bb_grad.Min.x + 1, y_pos) : ImVec2(bb_grad.Max.x - 1, y_pos),
					opposite ? ImVec2(bb_grad.Min.x + tick_width, y_pos) : ImVec2(bb_grad.Max.x - tick_width, y_pos),
					tick_col,
					tick_thick);
			}
			const float txt_x = opposite ? bb_grad.Min.x - txt_off - gp.CTicker.Ticks[i].LabelSize.x : bb_grad.Max.x + txt_off;
			const float txt_y = y_pos - gp.CTicker.Ticks[i].LabelSize.y * 0.5f;
			DrawList.AddText(ImVec2(txt_x, txt_y), col_text, gp.CTicker.GetText(i));
		}

		if (rend_label) {
			const float pos_x = opposite ? bb_frame.Min.x + gp.Style.PlotPadding.x : bb_grad.Max.x + 2 * txt_off + gp.CTicker.MaxSize.x;
			const float pos_y = bb_grad.GetCenter().y + label_size.x * 0.5f;
			const char* label_end = ImGui::FindRenderedTextEnd(label);
			AddTextVertical(&DrawList, ImVec2(pos_x, pos_y), col_text, label, label_end);
		}
		DrawList.AddRect(bb_grad.Min, bb_grad.Max, GetStyleColorU32(ImPlotCol_PlotBorder));
		float bars_triangles_half_sz = IM_TRUNC(bar_w * 0.20f);
		float y = bb_grad.Min.y + (bb_grad.Max.y - bb_grad.Min.y) * (scale_max - val) / (scale_max - scale_min);
		RenderArrowsForVerticalBar(&DrawList, ImVec2(bb_grad.Min.x - 1, y), ImVec2(bars_triangles_half_sz + 1, bars_triangles_half_sz), bar_w + 2.0f, 0.8);

	}
}