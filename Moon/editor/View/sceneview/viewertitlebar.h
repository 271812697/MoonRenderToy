#pragma once
#include <QToolBar>
namespace MOON {


	class ViewerWindowTitleBar : public QToolBar
	{
	public:
		explicit ViewerWindowTitleBar(QWidget* parent = nullptr);
		~ViewerWindowTitleBar();
	private:
		class ViewerWindowTitleBarInternal;
		ViewerWindowTitleBarInternal* mInternal = nullptr;
	};
}