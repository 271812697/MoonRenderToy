
#pragma once

#include <memory>

#include "tools/Event.h"

#include "UI/Panels/APanelTransformable.h"
#include "UI/Settings/PanelWindowSettings.h"

namespace UI::Panels
{

	class PanelWindow : public APanelTransformable
	{
	public:

		PanelWindow
		(
			const std::string& p_name = "",
			bool p_opened = true,
			const Settings::PanelWindowSettings& p_panelSettings = Settings::PanelWindowSettings{}
		);


		void Open();


		void Close();


		void Focus();


		void SetOpened(bool p_value);

		bool IsOpened() const;

		bool IsHovered() const;

		bool IsFocused() const;

		bool IsAppearing() const;

        void ScrollToBottom();

        void ScrollToTop();

        bool IsScrolledToBottom() const;

        bool IsScrolledToTop() const;

	protected:
        void virtual _Draw_ImplInWindow(){
			DrawInWindow.Invoke();
		};
		void _Draw_Impl() override;

	public:
		std::string name;

		Maths::FVector2 minSize = { 0.f, 0.f };
		Maths::FVector2 maxSize = { 0.f, 0.f };

		bool resizable = true;
		bool closable = false;
		bool movable = true;
		bool scrollable = true;
		bool dockable = false;
		bool hideBackground = false;
		bool forceHorizontalScrollbar = false;
		bool forceVerticalScrollbar = false;
		bool allowHorizontalScrollbar = false;
		bool bringToFrontOnFocus = true;
		bool collapsable = false;
		bool allowInputs = true;
		bool titleBar = true;


		Tools::Eventing::Event<> OpenEvent;
		Tools::Eventing::Event<> CloseEvent;
		Tools::Eventing::Event<> DrawInWindow;
	protected:
		bool m_opened;
		bool m_hovered;
		bool m_focused;
        bool m_mustScrollToBottom = false;
        bool m_mustScrollToTop = false;
        bool m_scrolledToBottom = false;
        bool m_scrolledToTop = false;
	};
}