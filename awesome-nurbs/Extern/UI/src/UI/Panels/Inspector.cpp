
#include"UI/Panels/Inspector.h"

using namespace UI::Widgets;
namespace UI::Panels {

	Inspector::Inspector
	(
		const std::string& p_title,
		bool p_opened,
		const UI::Settings::PanelWindowSettings& p_windowSettings
	) : PanelWindow(p_title, p_opened, p_windowSettings)
	{

		//InstallUI();
		init();
	}

	void Inspector::init()
	{
		InstallUI();
	}


	Inspector::~Inspector()
	{

	}
}