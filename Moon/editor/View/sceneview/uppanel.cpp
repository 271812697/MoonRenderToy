#include "uppanel.h"
#include "viewerwindow.h"
#include "titlebar.h"
#include <QVBoxLayout>

namespace MOON {
	UpPanel::UpPanel(QWidget* parent, Qt::WindowFlags f) :QWidget(parent)
	{
		auto sceneWindow = new ViewerWindow(this);
		auto titleBar = new ViewerWindowTitleBar(this);
		QVBoxLayout* layout = new QVBoxLayout(this);
		layout->setContentsMargins(0, 0, 0, 0);
		layout->setSpacing(0);
		layout->addWidget(titleBar);
		layout->addWidget(sceneWindow);
		layout->setStretch(1, 1);
		// default to strong focus
		this->setFocusPolicy(Qt::StrongFocus);
		this->setMouseTracking(true);
	}
	void UpPanel::keyPressEvent(QKeyEvent* event) {
	}
}