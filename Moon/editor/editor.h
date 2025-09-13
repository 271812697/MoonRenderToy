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
		class EditorInternal;
		EditorInternal* mInternal = nullptr;
	};

}