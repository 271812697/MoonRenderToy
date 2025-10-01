#pragma once
#include<QFrame>
namespace MOON {


	class ViewerPanel : public QWidget
	{
	public:
		explicit ViewerPanel(QWidget* parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());

		void keyPressEvent(QKeyEvent* event) override;
	};
}