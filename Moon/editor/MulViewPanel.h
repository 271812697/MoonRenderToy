#pragma once
#include <QWidget>
namespace MOON {


	class MulViewPanel : public QWidget
	{
	public:
		explicit MulViewPanel(QWidget* parent = nullptr);
		~MulViewPanel();
	private:
		class MulViewPanelImpl;

		MulViewPanelImpl* impl;
	};
}