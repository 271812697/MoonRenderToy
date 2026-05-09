#include "editor/UI/TaskPanel/TaskViewPanel.h"
#include "editor/UI/TaskPanel/TaskViewWidget.h"
namespace MOON {
	TaskViewPanel::TaskViewPanel(QWidget* parent) :QDockWidget(parent)
	{
		TaskViewWidget* ui = new TaskViewWidget(this);
		setWidget(ui);
	}
}