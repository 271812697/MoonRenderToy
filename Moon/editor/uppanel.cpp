#include <QVBoxLayout>
#include "uppanel.h"
#include "viewerwindow.h"
#include<iostream>

namespace MOON {



	UpPanel::UpPanel(QWidget* parent, Qt::WindowFlags f) :QWidget(parent)
	{
		auto preview_window_ = new ViewerWindow(this);
		QSizePolicy sizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
		sizePolicy.setHorizontalStretch(0);
		sizePolicy.setVerticalStretch(0);

		QVBoxLayout* layout = new QVBoxLayout(this);
		layout->addWidget(preview_window_);
		setFocusPolicy(Qt::StrongFocus);  // 允许通过点击或Tab键获取焦点
		setFocus();                      // 主动获取焦点（可选）

	}
	void UpPanel::keyPressEvent(QKeyEvent* event) {
		std::cout << "555" << std::endl;
	}

}