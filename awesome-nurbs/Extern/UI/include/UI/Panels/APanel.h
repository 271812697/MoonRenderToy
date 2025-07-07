
#pragma once

#include <vector>
#include <unordered_map>

#include "UI/Internal/WidgetContainer.h"

namespace UI::Panels
{

	class APanel : public API::IDrawable, public Internal::WidgetContainer
	{
	public:

		APanel();


		void Draw() override;

		const std::string& GetPanelID() const;

	protected:
		virtual void _Draw_Impl() = 0;

	public:
		bool enabled = true;

	protected:
		std::string m_panelID;

	private:
		static uint64_t __PANEL_ID_INCREMENT;
	};
}