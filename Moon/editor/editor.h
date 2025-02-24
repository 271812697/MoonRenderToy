#pragma once
#include <QSplitter>
#include<qmainwindow.h>

namespace MOON {
	class Editor : public QMainWindow
	{
		Q_OBJECT
	public:
		Editor();
		~Editor();
	private:
		void init_panels();
		QSplitter* hori_splitter_ = nullptr;
		QSplitter* vert_splitter_ = nullptr;
	};

}