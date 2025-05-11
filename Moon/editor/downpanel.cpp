#include <QVBoxLayout>
#include "downpanel.h"
#include "pathtracePanel.h"
#include <QPushButton>
namespace MOON {



	DownPanel::DownPanel(QWidget* parent, Qt::WindowFlags f) :QWidget(parent)
	{
		auto path_trace_window = new PathTracePanel(this);


		QVBoxLayout* layout = new QVBoxLayout(this);
		layout->addWidget(path_trace_window);
		layout->setContentsMargins(0, 0, 0, 0);

	}

}