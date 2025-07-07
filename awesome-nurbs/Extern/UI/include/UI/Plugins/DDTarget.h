#pragma once
#include <string>
#include "tools/Event.h"
#include "imgui.h"
#include "UI/Plugins/IPlugin.h"
namespace UI::Plugins
{

	template<typename T>
	class DDTarget : public IPlugin
	{
	public:

		DDTarget(const std::string& p_identifier) : identifier(p_identifier)
		{}


		virtual void Execute() override
		{
			if (ImGui::BeginDragDropTarget())
			{
				if (!m_isHovered)
					HoverStartEvent.Invoke();

				m_isHovered = true;

				ImGuiDragDropFlags target_flags = 0;
				// target_flags |= ImGuiDragDropFlags_AcceptBeforeDelivery;    // Don't wait until the delivery (release mouse button on a target) to do something
				
				if (!showYellowRect)
					target_flags |= ImGuiDragDropFlags_AcceptNoDrawDefaultRect; // Don't display the yellow rectangle

				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(identifier.c_str(), target_flags))
				{
					T data = *(T*)payload->Data;
					DataReceivedEvent.Invoke(data);
				}
				ImGui::EndDragDropTarget();
			}
			else
			{
				if (m_isHovered)
					HoverEndEvent.Invoke();

				m_isHovered = false;
			}
		}

		bool IsHovered() const
		{
			return m_isHovered;
		}

	public:
		std::string identifier;
		Tools::Eventing::Event<T> DataReceivedEvent;
		Tools::Eventing::Event<> HoverStartEvent;
		Tools::Eventing::Event<> HoverEndEvent;

		bool showYellowRect = true;

	private:
		bool m_isHovered;
	};
}