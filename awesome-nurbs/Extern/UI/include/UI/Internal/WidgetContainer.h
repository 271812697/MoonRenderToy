#pragma once
#include <vector>
#include "UI/Widgets/AWidget.h"
#include "UI/Internal/EMemoryMode.h"

namespace UI::Internal
{

	class WidgetContainer
	{
	public:

		void RemoveWidget(Widgets::AWidget& p_widget);
		

		void RemoveAllWidgets();

	
		void ConsiderWidget(Widgets::AWidget& p_widget, bool p_manageMemory = true);


		void UnconsiderWidget(Widgets::AWidget& p_widget);

		void CollectGarbages();


		void DrawWidgets();

 
        void ReverseDrawOrder(bool reversed = true);


		template <typename T, typename ... Args>
		T& CreateWidget(Args&&... p_args)
		{
			m_widgets.emplace_back(new T(p_args...), Internal::EMemoryMode::INTERNAL_MANAGMENT);
			T& instance = *reinterpret_cast<T*>(m_widgets.back().first);
			instance.SetParent(this);
			return instance;
		}

		std::vector<std::pair<UI::Widgets::AWidget*, Internal::EMemoryMode>>& GetWidgets();

	protected:
		std::vector<std::pair<UI::Widgets::AWidget*, Internal::EMemoryMode>> m_widgets;
        bool m_reversedDrawOrder = false;
	};
}