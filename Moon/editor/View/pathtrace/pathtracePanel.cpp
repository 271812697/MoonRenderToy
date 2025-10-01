#include "pathtraceWidget.h"
#include "pathtracePanel.h"
#include "pathtracetitlebar.h"
#include <QPushButton>
#include <QVBoxLayout>
namespace MOON {
	PathTracePanel::PathTracePanel(QWidget* parent, Qt::WindowFlags f) :QWidget(parent)
	{
		auto path_trace_window = new PathTraceWidget(this);
		auto titleBar = new PathTraceTitleBar(this);
		QVBoxLayout* layout = new QVBoxLayout(this);
		layout->setContentsMargins(0, 0, 0, 0);
		layout->setSpacing(0);
		layout->addWidget(titleBar);
		layout->addWidget(path_trace_window);
		layout->setStretch(1, 1);

	}
}