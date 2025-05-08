#pragma once
#include<QFrame>
namespace MOON {


	class UpPanel : public QWidget
	{
	public:
		explicit UpPanel(QWidget* parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());

		void keyPressEvent(QKeyEvent* event) override;
	};
}