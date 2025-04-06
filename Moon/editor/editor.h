#pragma once

#include<qmainwindow.h>
class QSplitter;
class QMenuBar;
class QMenu;
class QAction;
namespace MOON {
	class Editor : public QMainWindow
	{
		Q_OBJECT
	public:
		Editor();
		~Editor();
	private:
		void init_panels();
		void buildMenu();
		void retranslateUi();
		QSplitter* hori_splitter_ = nullptr;
		QSplitter* vert_splitter_ = nullptr;
		QMenuBar* mMenubar;
		QMenu* menu_File;
		QAction* openfile;
	};

}