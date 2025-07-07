#pragma once

#include <algorithm>
#include "tools/Event.h"
#include "UI/API/IDrawable.h"
#include "UI/Panels/APanel.h"
#include "imgui.h"
#include "ImGuizmo.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

namespace UI::Modules
{
	class Canvas : public API::IDrawable
	{
	public:

		void AddPanel(Panels::APanel& p_panel);


		void RemovePanel(Panels::APanel& p_panel);

		void RemoveAllPanels();

		void MakeDockspace(bool p_state);

		bool IsDockspace() const;


		void Draw() override;

	private:
		std::vector<std::reference_wrapper<Panels::APanel>> m_panels;
		bool m_isDockspace = false;
	};
}
