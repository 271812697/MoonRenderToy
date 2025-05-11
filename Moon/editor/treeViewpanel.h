#pragma once
#include <QtWidgets/QTreeView>
namespace MOON {
	class TreeViewPanel : public QTreeView
	{
	public:
		TreeViewPanel(QWidget* parent);
		void initModel();
	};
}