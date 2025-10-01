#include <qpushbutton.h>
#include <qboxlayout.h>
#include <QtWidgets/QApplication>
#include <QMenuBar>
#include <QSplitter>
#include <QtWidgets/QDockWidget>
#include "editor.h"
#include "hierarchypanel.h"
#include "MulViewPanel.h"
#include "UI/ReousrcePanel/resourcePanel.h"
#include "Debug/debugPanel.h"
#include "Reaction/pqLoadDataReaction.h"

namespace MOON {
	class Editor::EditorInternal {
	public:

		EditorInternal(Editor* editor) :self(editor)
		{

		}
		void initPanels() {
			QIcon icon;
			icon.addFile(QString::fromUtf8(":/widgets/icons/awesomeface.png"), QSize(), QIcon::Normal, QIcon::Off);
			self->setWindowIcon(icon);
			auto centralwidget = new QWidget(self);
			self->setCentralWidget(centralwidget);
			auto centralwidget_layout = new QHBoxLayout(centralwidget);
			auto middlePanel = new MulViewPanel(centralwidget);
			QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
			sizePolicy.setHeightForWidth(middlePanel->sizePolicy().hasHeightForWidth());
			middlePanel->setSizePolicy(sizePolicy);
			centralwidget_layout->addWidget(middlePanel);
			centralwidget_layout->setContentsMargins(0, 0, 0, 0);


			auto HierarchypanelDock = new QDockWidget(self);
			HierarchypanelDock->setAllowedAreas(Qt::AllDockWidgetAreas);
			auto hierarchypanel = new Hierarchypanel(HierarchypanelDock);
			HierarchypanelDock->setWidget(hierarchypanel);
			HierarchypanelDock->setWindowTitle(QApplication::translate("Hierarchypanel", "Hierarchy", nullptr));

			self->addDockWidget(Qt::LeftDockWidgetArea, HierarchypanelDock);
			auto resourcePanelDock = new ResPanel(self);
			self->addDockWidget(Qt::RightDockWidgetArea, resourcePanelDock);
			auto debugWidget = new DebugWidget(self);
			self->addDockWidget(Qt::LeftDockWidgetArea, debugWidget);
			self->tabifyDockWidget(HierarchypanelDock, debugWidget);

		}
		void buildMenu() {
			mMenubar = new QMenuBar(self);
			mMenubar->setObjectName(QString::fromUtf8("menubar"));
			//mMenubar->setGeometry(QRect(0, 0, 1152, 20));

			self->setMenuBar(mMenubar);
			menu_File = new QMenu(mMenubar);
			mMenubar->addAction(menu_File->menuAction());
			openfile = new QAction(menu_File);
			menu_File->addAction(openfile);
			QIcon icon9;
			icon9.addFile(QString::fromUtf8(":/widgets/icons/pqOpen.svg"), QSize(), QIcon::Normal, QIcon::Off);
			openfile->setIcon(icon9);
			new pqLoadDataReaction(openfile);
		}
		void retranslateUi() {
			openfile->setObjectName(QString::fromUtf8("actionFileOpen"));
			self->setWindowTitle(QCoreApplication::translate("Editor", "MainWindow", nullptr));
			menu_File->setTitle("&File");
			openfile->setText("&Open");
			openfile->setStatusTip("Open");
			openfile->setShortcut(QCoreApplication::translate("pqFileMenuBuilder", "Ctrl+O", nullptr));

		}
	private:
		Editor* self = nullptr;
		QSplitter* vert_splitter_ = nullptr;
		QMenuBar* mMenubar;
		QMenu* menu_File;
		QAction* openfile;
	};
	Editor::Editor() :mInternal(new EditorInternal(this))
	{
		mInternal->initPanels();
		mInternal->buildMenu();
		mInternal->retranslateUi();
	}
	Editor::~Editor()
	{
		delete mInternal;
	}

}