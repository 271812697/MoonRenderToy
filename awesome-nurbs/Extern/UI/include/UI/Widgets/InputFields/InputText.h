

#pragma once

#include <tools/Event.h>

#include "UI/Widgets/DataWidget.h"

namespace UI::Widgets
{
	namespace InputFields {
		class InputText : public DataWidget<std::string>
		{
		public:

			InputText(const std::string& p_content = "", const std::string& p_label = "");

		protected:
			void _Draw_Impl() override;

		public:
			std::string content;
			std::string label;
			bool selectAllOnClick = false;
			Tools::Eventing::Event<std::string> ContentChangedEvent;
			Tools::Eventing::Event<std::string> EnterPressedEvent;
		};
	}

}