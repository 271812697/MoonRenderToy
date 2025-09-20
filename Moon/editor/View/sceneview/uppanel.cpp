#include <QVBoxLayout>
#include "uppanel.h"
#include "viewerwindow.h"
#include<iostream>

namespace MOON {
	UpPanel::UpPanel(QWidget* parent, Qt::WindowFlags f) :QWidget(parent)
	{
		auto preview_window_ = new ViewerWindow(this);

		QVBoxLayout* layout = new QVBoxLayout(this);
		layout->addWidget(preview_window_);
		layout->setContentsMargins(0, 0, 0, 0);
		// default to strong focus
		this->setFocusPolicy(Qt::StrongFocus);
		this->setMouseTracking(true);
	}
	void UpPanel::keyPressEvent(QKeyEvent* event) {

	}

}