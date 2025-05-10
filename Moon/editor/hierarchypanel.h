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
		QWidget* up_panel = nullptr;
		QVBoxLayout* up_panel_layout = nullptr;
	private:
		QPushButton* create = nullptr;
		QPushButton* remove = nullptr;
		QPushButton* rename = nullptr;
		QPushButton* import_ = nullptr;

		TreeViewPanel* name_list = nullptr;
	};
}