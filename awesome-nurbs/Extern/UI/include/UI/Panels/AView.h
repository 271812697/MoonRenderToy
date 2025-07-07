#pragma once
#include <UI/Panels/PanelWindow.h>
#include <UI/Widgets/Visual/Image.h>
#include <UI/Panels/FrameBuffer.h>
namespace UI::Panels {
	class AView : public UI::Panels::PanelWindow
	{
	public:

		AView
		(
			const std::string& p_title,
			bool p_opened,
			const UI::Settings::PanelWindowSettings& p_windowSettings
		);

		virtual void Update(float p_deltaTime);

		void _Draw_Impl() override;

		virtual void _Render_Impl() {}

		void Render();

		std::pair<uint16_t, uint16_t> GetSafeSize() const;

		void Bind();

		void UnBind();

		Tools::Eventing::Event<uint16_t, uint16_t> ResizeEvent;

	protected:

		UI::Widgets::Visual::Image* m_image;
		std::unique_ptr<UI::Panels::Framebuffer> m_fbo;

	};
}
