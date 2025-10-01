#include <QVBoxLayout>
#include "pathtraceWidget.h"
#include "pathtracePanel.h"
#include <QPushButton>
namespace MOON {
	PathTracePanel::PathTracePanel(QWidget* parent, Qt::WindowFlags f) :QWidget(parent)
	{
		auto path_trace_window = new PathTraceWidget(this);
		QVBoxLayout* layout = new QVBoxLayout(this);
		layout->addWidget(path_trace_window);
		layout->setContentsMargins(0, 0, 0, 0);

	}
}