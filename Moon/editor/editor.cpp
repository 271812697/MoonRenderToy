#include <qpushbutton.h>
#include <qboxlayout.h>
#include <QtWidgets/QApplication>
#include <QMenuBar>
#include <QSplitter>
#include <QtWidgets/QDockWidget>
#include "editor.h"
#include "UI/TreeViewPanel/hierarchypanel.h"
#include "UI/SettingPanel/SettingPanel.h"
#include "MulViewPanel.h"
#include "UI/ReousrcePanel/resourcePanel.h"
#include "Debug/debugPanel.h"
#include "Command/menubar/openFile.h"
#include "Command/menubar/cameraMode.h"
#include "Command/menubar/visibleview.h"

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

			auto hierarchypanel = new Hierarchypanel(self);
			hierarchypanel->setAllowedAreas(Qt::AllDockWidgetAreas);
			hierarchypanel->setWindowTitle(QApplication::translate("Hierarchypanel", "Hierarchy", nullptr));
			
			auto settingPanel = new SettingPanel(self);
			settingPanel->setAllowedAreas(Qt::AllDockWidgetAreas);
			settingPanel->setWindowTitle(QApplication::translate("Settingpanel", "Setting", nullptr));
			
			self->addDockWidget(Qt::LeftDockWidgetArea, hierarchypanel);
			self->addDockWidget(Qt::LeftDockWidgetArea, settingPanel);
			auto resourcePanelDock = new ResPanel(self);
			self->addDockWidget(Qt::RightDockWidgetArea, resourcePanelDock);
			//auto debugWidget = new DebugWidget(self);
			//self->addDockWidget(Qt::LeftDockWidgetArea, debugWidget);
			//self->tabifyDockWidget(HierarchypanelDock, debugWidget);

		}
		void buildFileMenu() {
			auto openFileCommand=new OpenFileCommand(self);
			menu_File->addAction(openFileCommand->action());
		}
		void buildDisplayMenu() {
			auto cameraModeCommand = new CameraModeComand(self);
			menu_Display->addAction(cameraModeCommand->action());
		/*	QIcon icon;
			icon.addFile(QString::fromUtf8(":/widgets/icons/points.png"), QSize(), QIcon::Normal, QIcon::On);
			auto openfile = new QAction(self);
			
			openfile->setObjectName(QString::fromUtf8("actionFileOpen"));
			openfile->setText("Open");
			openfile->setStatusTip("Open");
			openfile->setShortcut(QCoreApplication::translate("pqFileMenuBuilder", "Ctrl+O", nullptr));
		
			openfile->setIcon(icon);
			menu_Display->addAction(openfile);*/
		}
		void buildViewMenu() {
			auto visible=new VisibleViewCommand(menu_View);
			visible->setUp(menu_View);
		}
		void buildMenu() {
			mMenubar = new QMenuBar(self);
			mMenubar->setObjectName(QString::fromUtf8("menubar"));
			//mMenubar->setGeometry(QRect(0, 0, 1152, 20));

			self->setMenuBar(mMenubar);
			menu_File = new QMenu(mMenubar);
			menu_Display = new QMenu(mMenubar);
			menu_View = new QMenu(mMenubar);
			mMenubar->addAction(menu_File->menuAction());
			mMenubar->addAction(menu_Display->menuAction());
			mMenubar->addAction(menu_View->menuAction());
			buildFileMenu();
			buildDisplayMenu();
			buildViewMenu();
	
		}
		void retranslateUi() {
			
			self->setWindowTitle(QCoreApplication::translate("Editor", "MainWindow", nullptr));
			menu_File->setTitle("&File");
			menu_Display->setTitle("&Display");
			menu_View->setTitle("&View");
		
		}
	private:
		Editor* self = nullptr;
		QSplitter* vert_splitter_ = nullptr;
		QMenuBar* mMenubar;
		QMenu* menu_File;
		QMenu* menu_Display;
		QMenu* menu_View;
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