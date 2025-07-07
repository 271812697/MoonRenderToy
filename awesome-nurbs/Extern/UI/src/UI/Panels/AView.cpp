#include"Maths/FVector2.h"
#include "UI/Panels/AView.h"
#include<glad/glad.h>

UI::Panels::AView::AView
(
	const std::string& p_title,
	bool p_opened,
	const UI::Settings::PanelWindowSettings& p_windowSettings
) : PanelWindow(p_title, p_opened, p_windowSettings)
{
	m_fbo = std::make_unique<Framebuffer>(1, 1);
	
	m_image = &CreateWidget<UI::Widgets::Visual::Image>(m_fbo->GetTextureID(), Maths::FVector2{ 0.f, 0.f });
	scrollable = false;
}

void UI::Panels::AView::Update(float p_deltaTime)
{
	auto [winWidth, winHeight] = GetSafeSize();

	m_image->size = Maths::FVector2(static_cast<float>(winWidth), static_cast<float>(winHeight));

	if (winWidth == 0 || winHeight == 0)
		return;

	if (m_fbo->width != winWidth || m_fbo->height != winHeight) {

		m_fbo->Resize(winWidth, winHeight);
		ResizeEvent.Invoke(winWidth, winHeight);
	}

}

void UI::Panels::AView::_Draw_Impl()
{
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));

	UI::Panels::PanelWindow::_Draw_Impl();

	ImGui::PopStyleVar();
}

void UI::Panels::AView::Render()
{
	auto [winWidth, winHeight] = GetSafeSize();
	_Render_Impl();
}

std::pair<uint16_t, uint16_t> UI::Panels::AView::GetSafeSize() const
{
	auto result = GetSize();
	result.y -= 25.0f;
	return { static_cast<uint16_t>(result.x), static_cast<uint16_t>(result.y) };
}

void UI::Panels::AView::Bind()
{
	
	m_fbo->Bind();
	glViewport(0, 0, m_fbo->width, m_fbo->height);
}

void UI::Panels::AView::UnBind()
{
	m_fbo->Unbind();
}
