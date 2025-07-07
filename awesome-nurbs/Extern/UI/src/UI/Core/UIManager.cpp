#include "UI/Core/UIManager.h"

UI::Core::UIManager::UIManager(GLFWwindow* p_glfwWindow, Styling::EStyle p_style, const std::string& p_glslVersion)
{
	ImGui::CreateContext();

	ImGui::GetIO().ConfigWindowsMoveFromTitleBarOnly = true; /* Disable moving windows by dragging another thing than the title bar */
	EnableDocking(false);

	ApplyStyle(p_style);

	ImGui_ImplGlfw_InitForOpenGL(p_glfwWindow, true);
	ImGui_ImplOpenGL3_Init(p_glslVersion.c_str());
}

UI::Core::UIManager::~UIManager()
{
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}

void UI::Core::UIManager::ApplyStyle(Styling::EStyle p_style)
{
	ImGuiStyle* style = &ImGui::GetStyle();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
	switch (p_style)
	{
	case UI::Styling::EStyle::IM_CLASSIC_STYLE:	    ImGui::StyleColorsClassic();	break;
	case UI::Styling::EStyle::IM_DARK_STYLE:		ImGui::StyleColorsDark();		break;
	case UI::Styling::EStyle::IM_LIGHT_STYLE:		ImGui::StyleColorsLight();		break;

	}

	if (p_style == UI::Styling::EStyle::DUNE_DARK)
	{
		style->WindowPadding = ImVec2(15, 15);
		style->WindowRounding = 5.0f;
		style->FramePadding = ImVec2(5, 5);
		style->FrameRounding = 4.0f;
		style->ItemSpacing = ImVec2(12, 8);
		style->ItemInnerSpacing = ImVec2(8, 6);
		style->IndentSpacing = 25.0f;
		style->ScrollbarSize = 15.0f;
		style->ScrollbarRounding = 9.0f;
		style->GrabMinSize = 5.0f;
		style->GrabRounding = 3.0f;

		style->Colors[ImGuiCol_Text] = ImVec4(0.80f, 0.80f, 0.83f, 1.00f);
		style->Colors[ImGuiCol_TextDisabled] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
		style->Colors[ImGuiCol_WindowBg] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
		style->Colors[ImGuiCol_ChildBg] = ImVec4(0.07f, 0.07f, 0.09f, 1.00f);
		style->Colors[ImGuiCol_PopupBg] = ImVec4(0.07f, 0.07f, 0.09f, 1.00f);
		style->Colors[ImGuiCol_Border] = ImVec4(0.2f, 0.2f, 0.2f, 0.88f);
		style->Colors[ImGuiCol_BorderShadow] = ImVec4(0.92f, 0.91f, 0.88f, 0.00f);
		style->Colors[ImGuiCol_FrameBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
		style->Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
		style->Colors[ImGuiCol_FrameBgActive] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
		style->Colors[ImGuiCol_TitleBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
		style->Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.3f, 0.3f, 0.3f, 0.75f);
		style->Colors[ImGuiCol_TitleBgActive] = ImVec4(0.07f, 0.07f, 0.09f, 1.00f);
		style->Colors[ImGuiCol_MenuBarBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
		style->Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
		style->Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.80f, 0.80f, 0.83f, 0.31f);
		style->Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
		style->Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
		style->Colors[ImGuiCol_CheckMark] = ImVec4(0.80f, 0.80f, 0.83f, 0.31f);
		style->Colors[ImGuiCol_SliderGrab] = ImVec4(0.80f, 0.80f, 0.83f, 0.31f);
		style->Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
		style->Colors[ImGuiCol_Button] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
		style->Colors[ImGuiCol_ButtonHovered] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
		style->Colors[ImGuiCol_ButtonActive] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
		style->Colors[ImGuiCol_Header] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
		style->Colors[ImGuiCol_HeaderHovered] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
		style->Colors[ImGuiCol_HeaderActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
		style->Colors[ImGuiCol_Separator] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
		style->Colors[ImGuiCol_SeparatorHovered] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
		style->Colors[ImGuiCol_SeparatorActive] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
		style->Colors[ImGuiCol_ResizeGrip] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
		style->Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
		style->Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
		style->Colors[ImGuiCol_PlotLines] = ImVec4(0.40f, 0.39f, 0.38f, 0.63f);
		style->Colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.25f, 1.00f, 0.00f, 1.00f);
		style->Colors[ImGuiCol_PlotHistogram] = ImVec4(0.40f, 0.39f, 0.38f, 0.63f);
		style->Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.25f, 1.00f, 0.00f, 1.00f);
		style->Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.25f, 1.00f, 0.00f, 0.43f);
		//style->Colors[ImGuiCol_ModalWindowDarkening] = ImVec4(1.00f, 0.98f, 0.95f, 0.73f);

		style->Colors[ImGuiCol_Tab] = style->Colors[ImGuiCol_TabUnfocused];
	}
	else if (p_style == UI::Styling::EStyle::ALTERNATIVE_DARK)
	{
		style->WindowPadding = ImVec2(15, 15);
		style->WindowRounding = 0.0f;
		style->FramePadding = ImVec2(5, 5);
		style->FrameRounding = 0.0f;
		style->ItemSpacing = ImVec2(12, 8);
		style->ItemInnerSpacing = ImVec2(8, 6);
		style->IndentSpacing = 25.0f;
		style->ScrollbarSize = 15.0f;
		style->ScrollbarRounding = 0.0f;
		style->GrabMinSize = 5.0f;
		style->GrabRounding = 0.0f;
		style->TabRounding = 0.0f;
		style->ChildRounding = 0.0f;
		style->PopupRounding = 0.0f;

		style->WindowBorderSize = 1.0f;
		style->FrameBorderSize = 0.0f;
		style->PopupBorderSize = 1.0f;

		ImVec4* colors = ImGui::GetStyle().Colors;
		colors[ImGuiCol_Text] = ImVec4(0.96f, 0.96f, 0.99f, 1.00f);
		colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
		colors[ImGuiCol_WindowBg] = ImVec4(0.09f, 0.09f, 0.10f, 1.00f);
		colors[ImGuiCol_ChildBg] = ImVec4(0.09f, 0.09f, 0.10f, 1.00f);
		colors[ImGuiCol_PopupBg] = ImVec4(0.06f, 0.06f, 0.07f, 1.00f);
		colors[ImGuiCol_Border] = ImVec4(0.12f, 0.12f, 0.14f, 1.00f);
		colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
		colors[ImGuiCol_FrameBg] = ImVec4(0.12f, 0.12f, 0.13f, 1.00f);
		colors[ImGuiCol_FrameBgHovered] = ImVec4(0.20f, 0.20f, 0.22f, 1.00f);
		colors[ImGuiCol_FrameBgActive] = ImVec4(0.27f, 0.27f, 0.29f, 1.00f);
		colors[ImGuiCol_TitleBg] = ImVec4(0.07f, 0.07f, 0.07f, 1.00f);
		colors[ImGuiCol_TitleBgActive] = ImVec4(0.07f, 0.07f, 0.07f, 1.00f);
		colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.07f, 0.07f, 0.07f, 1.00f);
		colors[ImGuiCol_MenuBarBg] = ImVec4(0.07f, 0.07f, 0.07f, 1.00f);
		colors[ImGuiCol_ScrollbarBg] = ImVec4(0.07f, 0.07f, 0.07f, 1.00f);
		colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.31f, 0.31f, 0.32f, 1.00f);
		colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.41f, 0.41f, 0.42f, 1.00f);
		colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.51f, 0.51f, 0.53f, 1.00f);
		colors[ImGuiCol_CheckMark] = ImVec4(0.44f, 0.44f, 0.47f, 1.00f);
		colors[ImGuiCol_SliderGrab] = ImVec4(0.44f, 0.44f, 0.47f, 1.00f);
		colors[ImGuiCol_SliderGrabActive] = ImVec4(0.59f, 0.59f, 0.61f, 1.00f);
		colors[ImGuiCol_Button] = ImVec4(0.20f, 0.20f, 0.22f, 1.00f);
		colors[ImGuiCol_ButtonHovered] = ImVec4(0.44f, 0.44f, 0.47f, 1.00f);
		colors[ImGuiCol_ButtonActive] = ImVec4(0.59f, 0.59f, 0.61f, 1.00f);
		colors[ImGuiCol_Header] = ImVec4(0.20f, 0.20f, 0.22f, 1.00f);
		colors[ImGuiCol_HeaderHovered] = ImVec4(0.44f, 0.44f, 0.47f, 1.00f);
		colors[ImGuiCol_HeaderActive] = ImVec4(0.59f, 0.59f, 0.61f, 1.00f);
		colors[ImGuiCol_Separator] = ImVec4(1.00f, 1.00f, 1.00f, 0.20f);
		colors[ImGuiCol_SeparatorHovered] = ImVec4(0.44f, 0.44f, 0.47f, 0.39f);
		colors[ImGuiCol_SeparatorActive] = ImVec4(0.44f, 0.44f, 0.47f, 0.59f);
		colors[ImGuiCol_ResizeGrip] = ImVec4(0.26f, 0.59f, 0.98f, 0.00f);
		colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.00f);
		colors[ImGuiCol_ResizeGripActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.00f);
		colors[ImGuiCol_Tab] = ImVec4(0.20f, 0.20f, 0.22f, 1.00f);
		colors[ImGuiCol_TabHovered] = ImVec4(0.44f, 0.44f, 0.47f, 1.00f);
		colors[ImGuiCol_TabActive] = ImVec4(0.44f, 0.44f, 0.47f, 1.00f);
		colors[ImGuiCol_TabUnfocused] = ImVec4(0.20f, 0.20f, 0.22f, 0.39f);
		colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.44f, 0.44f, 0.47f, 0.39f);
		colors[ImGuiCol_DockingPreview] = ImVec4(0.91f, 0.62f, 0.00f, 0.78f);
		colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
		colors[ImGuiCol_PlotLines] = ImVec4(0.96f, 0.96f, 0.99f, 1.00f);
		colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.12f, 1.00f, 0.12f, 1.00f);
		colors[ImGuiCol_PlotHistogram] = ImVec4(0.96f, 0.96f, 0.99f, 1.00f);
		colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.12f, 1.00f, 0.12f, 1.00f);
		colors[ImGuiCol_TextSelectedBg] = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
		colors[ImGuiCol_DragDropTarget] = ImVec4(0.91f, 0.62f, 0.00f, 1.00f);
		colors[ImGuiCol_NavHighlight] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
		colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
		colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
		colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);
	}
	else if (p_style == UI::Styling::EStyle::CUSTOM) {

		ImGuiStyle& style = ImGui::GetStyle();

		// load fonts from the resource folder
		float fontsize_main = 18.0f;
		float fontsize_icon = 18.0f;  // bake icon font into the main font
		float fontsize_sub = 17.0f;
		ImFontConfig config_main;
		config_main.PixelSnapH = true;
		config_main.OversampleH = 4;
		config_main.OversampleV = 4;
		config_main.RasterizerMultiply = 1.2f;  // brighten up the font to make them more readable
		config_main.GlyphExtraSpacing.x = 0.0f;

		ImFontConfig config_sub;
		config_sub.PixelSnapH = true;
		config_sub.OversampleH = 4;
		config_sub.OversampleV = 4;
		config_sub.RasterizerMultiply = 1.25f;  // brighten up the font to make them more readable
		config_sub.GlyphExtraSpacing.x = 0.0f;

		ImFontConfig config_icon;
		config_icon.MergeMode = true;
		config_icon.PixelSnapH = true;
		config_icon.OversampleH = 4;
		config_icon.OversampleV = 4;
		config_icon.RasterizerMultiply = 1.5f;  // brighten up the font to make them more readable
		config_icon.GlyphOffset.y = 0.0f;       // tweak this to vertically align with the main font
		config_icon.GlyphMinAdvanceX = fontsize_main;  // enforce monospaced icon font
		config_icon.GlyphMaxAdvanceX = fontsize_main;  // enforce monospaced icon font

		// load default dark theme
		ImGui::StyleColorsDark();
		// setup custom styles
		style.WindowBorderSize = 0.0f;
		style.FrameBorderSize = 1.0f;
		style.PopupBorderSize = 1.0f;
		style.ChildBorderSize = 1.0f;
		style.TabBorderSize = 0.0f;
		style.ScrollbarSize = 18.0f;
		style.GrabMinSize = 10.0f;

		style.WindowPadding = ImVec2(8.0f, 8.0f);
		style.FramePadding = ImVec2(4.0f, 6.0f);
		style.ItemSpacing = ImVec2(10.0f, 10.0f);
		style.ItemInnerSpacing = ImVec2(10.0f, 10.0f);
		style.IndentSpacing = 16.0f;

		style.WindowRounding = 0.0f;
		style.ChildRounding = 0.0f;
		style.FrameRounding = 4.0f;
		style.PopupRounding = 0.0f;
		style.TabRounding = 4.0f;
		style.GrabRounding = 4.0f;
		style.ScrollbarRounding = 12.0f;

		style.WindowMenuButtonPosition = ImGuiDir_Left;
		style.ColorButtonPosition = ImGuiDir_Right;

		style.ButtonTextAlign = ImVec2(0.5f, 0.5f);
		style.WindowTitleAlign = ImVec2(0.0f, 0.5f);
		style.SelectableTextAlign = ImVec2(0.0f, 0.0f);

		style.AntiAliasedLines = true;
		style.AntiAliasedFill = true;
		style.AntiAliasedLinesUseTex = true;

		// setup custom colors
		auto& c = ImGui::GetStyle().Colors;

		c[ImGuiCol_WindowBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.85f);
		c[ImGuiCol_ChildBg] = ImVec4(0.1f, 0.1f, 0.1f, 0.85f);
		c[ImGuiCol_PopupBg] = ImVec4(0.1f, 0.1f, 0.1f, 0.85f);

		c[ImGuiCol_FrameBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.75f);
		c[ImGuiCol_FrameBgHovered] = ImVec4(0.2f, 0.2f, 0.2f, 0.75f);
		c[ImGuiCol_FrameBgActive] = ImVec4(0.3f, 0.3f, 0.3f, 0.75f);

		c[ImGuiCol_TitleBg] = ImVec4(0.2f, 0.2f, 0.2f, 0.75f);
		c[ImGuiCol_TitleBgActive] = ImVec4(0.0f, 0.3f, 0.0f, 0.9f);
		c[ImGuiCol_TitleBgCollapsed] = ImVec4(0.0f, 0.0f, 0.0f, 0.75f);

		c[ImGuiCol_ScrollbarBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.75f);
		c[ImGuiCol_ScrollbarGrab] = ImVec4(0.2f, 0.2f, 0.2f, 0.9f);
		c[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.3f, 0.3f, 0.3f, 0.9f);
		c[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.4f, 0.4f, 0.4f, 0.9f);

		c[ImGuiCol_CheckMark] = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);
		c[ImGuiCol_SliderGrab] = ImVec4(0.0f, 0.4f, 0.0f, 0.9f);
		c[ImGuiCol_SliderGrabActive] = ImVec4(0.0f, 0.5f, 0.0f, 0.9f);

		c[ImGuiCol_Button] = ImVec4(0.0f, 0.3f, 0.0f, 0.9f);
		c[ImGuiCol_ButtonHovered] = ImVec4(0.0f, 0.55f, 0.0f, 0.9f);
		c[ImGuiCol_ButtonActive] = ImVec4(0.0f, 0.5f, 0.0f, 0.9f);

		c[ImGuiCol_Header] = ImVec4(0.5f, 0.0f, 1.0f, 0.5f);
		c[ImGuiCol_HeaderHovered] = ImVec4(0.5f, 0.0f, 1.0f, 0.8f);
		c[ImGuiCol_HeaderActive] = ImVec4(0.5f, 0.0f, 1.0f, 0.7f);

		c[ImGuiCol_Tab] = ImVec4(0.0f, 0.3f, 0.0f, 0.8f);
		c[ImGuiCol_TabHovered] = ImVec4(0.0f, 0.4f, 0.0f, 0.8f);
		c[ImGuiCol_TabActive] = ImVec4(0.0f, 0.4f, 0.0f, 0.8f);
		c[ImGuiCol_TabUnfocused] = ImVec4(0.2f, 0.2f, 0.2f, 0.9f);
		c[ImGuiCol_TabUnfocusedActive] = ImVec4(0.2f, 0.2f, 0.2f, 0.9f);
	}
}

bool UI::Core::UIManager::LoadFont(const std::string& p_id, const std::string& p_path, float p_fontSize)
{
	if (m_fonts.find(p_id) == m_fonts.end())
	{
		auto& io = ImGui::GetIO();
		ImFont* fontInstance = io.Fonts->AddFontFromFileTTF(p_path.c_str(), p_fontSize);

		if (fontInstance)
		{
			m_fonts[p_id] = fontInstance;
			return true;
		}
	}

	return false;
}

bool UI::Core::UIManager::UnloadFont(const std::string& p_id)
{
	if (m_fonts.find(p_id) != m_fonts.end())
	{
		m_fonts.erase(p_id);
		return true;
	}

	return false;
}

bool UI::Core::UIManager::UseFont(const std::string& p_id)
{
	auto foundFont = m_fonts.find(p_id);

	if (foundFont != m_fonts.end())
	{
		ImGui::GetIO().FontDefault = foundFont->second;
		return true;
	}

	return false;
}

void UI::Core::UIManager::UseDefaultFont()
{
	ImGui::GetIO().FontDefault = nullptr;
}

void UI::Core::UIManager::EnableEditorLayoutSave(bool p_value)
{
	if (p_value)
		ImGui::GetIO().IniFilename = m_layoutSaveFilename.c_str();
	else
		ImGui::GetIO().IniFilename = nullptr;
}

bool UI::Core::UIManager::IsEditorLayoutSaveEnabled() const
{
	return ImGui::GetIO().IniFilename != nullptr;
}

void UI::Core::UIManager::SetEditorLayoutSaveFilename(const std::string& p_filename)
{
	m_layoutSaveFilename = p_filename;
	if (IsEditorLayoutSaveEnabled())
		ImGui::GetIO().IniFilename = m_layoutSaveFilename.c_str();
}

void UI::Core::UIManager::SetEditorLayoutAutosaveFrequency(float p_frequency)
{
	ImGui::GetIO().IniSavingRate = p_frequency;
}

float UI::Core::UIManager::GetEditorLayoutAutosaveFrequency(float p_frequeny)
{
	return ImGui::GetIO().IniSavingRate;
}

void UI::Core::UIManager::EnableDocking(bool p_value)
{
	m_dockingState = p_value;

	if (p_value)
		ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	else
		ImGui::GetIO().ConfigFlags &= ~ImGuiConfigFlags_DockingEnable;
}

void UI::Core::UIManager::ResetLayout(const std::string& p_config) const
{
	ImGui::LoadIniSettingsFromDisk(p_config.c_str());
}

bool UI::Core::UIManager::IsDockingEnabled() const
{
	return m_dockingState;
}

void UI::Core::UIManager::SetCanvas(Modules::Canvas& p_canvas)
{
	RemoveCanvas();

	m_currentCanvas = &p_canvas;
}

void UI::Core::UIManager::RemoveCanvas()
{
	m_currentCanvas = nullptr;
}

void UI::Core::UIManager::Render()
{
	if (m_currentCanvas)
	{
		m_currentCanvas->Draw();
	}
}

void UI::Core::UIManager::PushCurrentFont()
{

}

void UI::Core::UIManager::PopCurrentFont()
{

}