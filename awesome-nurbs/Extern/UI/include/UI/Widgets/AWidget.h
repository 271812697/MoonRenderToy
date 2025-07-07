#pragma once
#include <string>

#include "UI/API/IDrawable.h"
#include "UI/Plugins/Pluginable.h"
#include "UI/Plugins/DataDispatcher.h"
#include "imgui.h"

namespace  UI::Internal { class WidgetContainer; }

namespace UI::Widgets
{
	class AWidget : public API::IDrawable, public Plugins::Pluginable
	{
	public:
		AWidget();

		virtual void Draw() override;

		void LinkTo(const AWidget& p_widget);

		void Destroy();

		bool IsDestroyed() const;

		void SetParent(Internal::WidgetContainer* p_parent);

		bool HasParent() const;

		Internal::WidgetContainer* GetParent();

	protected:
		virtual void _Draw_Impl() = 0;

	public:
		bool enabled = true;
		bool lineBreak = true;

	protected:
		Internal::WidgetContainer* m_parent;
		std::string m_widgetID = "?";
		bool m_autoExecutePlugins = true;

	private:
		static uint64_t __WIDGET_ID_INCREMENT;
		bool m_destroyed = false;
	};
}