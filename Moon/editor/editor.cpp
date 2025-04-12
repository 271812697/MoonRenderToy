#include <qpushbutton.h>
#include <qboxlayout.h>
#include <QtWidgets/QApplication>
#include <QMenuBar>
#include <QSplitter>
#include "editor.h"
#include "hierarchypanel.h"
#include "nodepanel.h"
#include "uppanel.h"
#include "pqLoadDataReaction.h"

namespace MOON {
	Editor::Editor()
	{
		init_panels();
	}
	Editor::~Editor()
	{
	}
	void Editor::init_panels()
	{
		hori_splitter_ = new QSplitter(this);
		auto centralwidget = new QWidget(this);
		QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
		sizePolicy.setHorizontalStretch(0);
		sizePolicy.setVerticalStretch(0);
		sizePolicy.setHeightForWidth(centralwidget->sizePolicy().hasHeightForWidth());
		centralwidget->setSizePolicy(sizePolicy);
		setCentralWidget(centralwidget);
		auto centralwidget_layout = new QHBoxLayout(centralwidget);
		centralwidget_layout->addWidget(hori_splitter_);
		QSizePolicy sizePolicy1(QSizePolicy::Expanding, QSizePolicy::Expanding);
		sizePolicy1.setHorizontalStretch(0);
		sizePolicy1.setVerticalStretch(0);
		sizePolicy1.setHeightForWidth(hori_splitter_->sizePolicy().hasHeightForWidth());
		hori_splitter_->setSizePolicy(sizePolicy1);
		hori_splitter_->setOrientation(Qt::Horizontal);


		QSizePolicy sizePolicy2(QSizePolicy::Minimum, QSizePolicy::Minimum);
		sizePolicy2.setHorizontalStretch(0);
		sizePolicy2.setVerticalStretch(0);


		//left_panel_ = new LeftPanel(hori_splitter_);
		auto hierarchypanel = new Hierarchypanel(hori_splitter_);
		vert_splitter_ = new QSplitter(hori_splitter_);
		auto right_panel_ = new QPushButton(hori_splitter_);
		sizePolicy2.setHeightForWidth(right_panel_->sizePolicy().hasHeightForWidth());
		//right_panel_ = new RightPanel(hori_splitter_);
		right_panel_->setSizePolicy(sizePolicy2);

		hori_splitter_->setOrientation(Qt::Horizontal);
		hori_splitter_->addWidget(hierarchypanel);
		hori_splitter_->addWidget(vert_splitter_);
		hori_splitter_->addWidget(right_panel_);
		auto up_panel_ = new UpPanel(vert_splitter_);
		auto down_panel_ = new Nodepanel(vert_splitter_);
		//up_panel_->setFrameShape(QFrame::Box);
		vert_splitter_->setOrientation(Qt::Vertical);
		vert_splitter_->addWidget(up_panel_);
		vert_splitter_->addWidget(down_panel_);
		buildMenu();

	}
	void Editor::buildMenu()
	{

		mMenubar = new QMenuBar(this);
		mMenubar->setObjectName(QString::fromUtf8("menubar"));
		mMenubar->setGeometry(QRect(0, 0, 1152, 20));

		this->setMenuBar(mMenubar);
		menu_File = new QMenu(mMenubar);
		mMenubar->addAction(menu_File->menuAction());
		openfile = new QAction(menu_File);
		menu_File->addAction(openfile);
		QIcon icon9;
		icon9.addFile(QString::fromUtf8(":/widgets/icons/pqOpen.svg"), QSize(), QIcon::Normal, QIcon::Off);
		openfile->setIcon(icon9);
		new pqLoadDataReaction(openfile);

		retranslateUi();
	}
	void Editor::retranslateUi()
	{
		openfile->setObjectName(QString::fromUtf8("actionFileOpen"));
		this->setWindowTitle(QCoreApplication::translate("Editor", "MainWindow", nullptr));
		menu_File->setTitle("&File");
		openfile->setText("&Open");
		openfile->setStatusTip("Open");
		openfile->setShortcut(QCoreApplication::translate("pqFileMenuBuilder", "Ctrl+O", nullptr));

	}
}