#pragma once
#include <QWidget>
namespace MOON {


	class ViewerWindowTitleBar : public QWidget
	{
	public:
		explicit ViewerWindowTitleBar(QWidget* parent = nullptr);
		~ViewerWindowTitleBar();
	private:
		class ViewerWindowTitleBarInternal;
		ViewerWindowTitleBarInternal* mInternal = nullptr;
	};
}