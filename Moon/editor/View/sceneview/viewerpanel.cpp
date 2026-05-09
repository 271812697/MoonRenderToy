#include "viewerpanel.h"
#include "viewerwidget.h"

#include <QVBoxLayout>

namespace MOON {
	ViewerPanel::ViewerPanel(QWidget* parent, Qt::WindowFlags f) :QWidget(parent)
	{
		auto sceneWindow = new ViewerWidget(this);
		
		QVBoxLayout* layout = new QVBoxLayout(this);
		layout->setContentsMargins(0, 0, 0, 0);
		layout->setSpacing(0);
		layout->addWidget(sceneWindow);
		// default to strong focus
		this->setFocusPolicy(Qt::StrongFocus);
		this->setMouseTracking(true);
	}
	void ViewerPanel::keyPressEvent(QKeyEvent* event) {
	}
}