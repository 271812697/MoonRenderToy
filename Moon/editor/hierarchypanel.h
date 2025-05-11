#pragma once
#include <QWidget>

class QPushButton;
class QVBoxLayout;
namespace MOON {
	class TreeViewPanel;
	class Hierarchypanel : public QWidget
	{
	public:
		Hierarchypanel(QWidget* parent);
	private:
		TreeViewPanel* name_list = nullptr;
	};
}