#include "editor/UI/TaskPanel/TaskViewPanel.h"
#include "editor/UI/TaskPanel/TaskViewWidget.h"
#include "editor/UI/DockWidgetTitleBar.h"
#include "Core/Global/ServiceLocator.h"
namespace MOON {
	TaskViewPanel::TaskViewPanel(QWidget* parent) :QDockWidget(parent)
	{
		RegService(TaskViewPanel, *this);
		setTitleBarWidget(new DockWidgetTitleBar(this));
		TaskViewWidget* ui = new TaskViewWidget(this);
		setWidget(ui);
	}
}
