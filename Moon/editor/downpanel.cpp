#include <QVBoxLayout>
#include "downpanel.h"
#include "pathtracePanel.h"
#include <QPushButton>
namespace MOON {



	DownPanel::DownPanel(QWidget* parent, Qt::WindowFlags f) :QWidget(parent)
	{
		auto path_trace_window = new PathTracePanel(this);
		QSizePolicy sizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
		sizePolicy.setHorizontalStretch(0);
		sizePolicy.setVerticalStretch(0);

		QVBoxLayout* layout = new QVBoxLayout(this);
		layout->addWidget(path_trace_window);

	}

}