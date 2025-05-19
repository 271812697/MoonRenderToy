#include <qpushbutton.h>
#include <qboxlayout.h>
#include <QtWidgets/QApplication>
#include <QMenuBar>
#include <QSplitter>
#include <QtWidgets/QDockWidget>
#include "editor.h"
#include "hierarchypanel.h"
#include "MulViewPanel.h"
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
		//hori_splitter_ = new QSplitter(this);
		//Middle Panel
		auto centralwidget = new QWidget(this);
		setCentralWidget(centralwidget);
		auto centralwidget_layout = new QHBoxLayout(centralwidget);
		auto middlePanel = new MulViewPanel(centralwidget);
		QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
		sizePolicy.setHorizontalStretch(120);
		sizePolicy.setVerticalStretch(1);
		sizePolicy.setHeightForWidth(middlePanel->sizePolicy().hasHeightForWidth());
		middlePanel->setSizePolicy(sizePolicy);
		centralwidget_layout->addWidget(middlePanel);
		centralwidget_layout->setContentsMargins(0, 0, 0, 0);


		auto HierarchypanelDock = new QDockWidget(this);
		HierarchypanelDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
		auto hierarchypanel = new Hierarchypanel(HierarchypanelDock);
		HierarchypanelDock->setWidget(hierarchypanel);
		this->addDockWidget(Qt::LeftDockWidgetArea, HierarchypanelDock);


		//auto right_panel_ = new QPushButton(hori_splitter_);
		//hori_splitter_->setOrientation(Qt::Horizontal);
		//hori_splitter_->addWidget(hierarchypanel);
		//hori_splitter_->addWidget(middlePanel);
		//hori_splitter_->addWidget(right_panel_);

		buildMenu();
	}
	void Editor::buildMenu()
	{

		mMenubar = new QMenuBar(this);
		mMenubar->setObjectName(QString::fromUtf8("menubar"));
		//mMenubar->setGeometry(QRect(0, 0, 1152, 20));

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