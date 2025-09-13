#pragma once

#include <QDockWidget>
namespace MOON {


	class DebugWidget : public QDockWidget
	{
		Q_OBJECT

	public:
		DebugWidget(QWidget* parent = 0);
		~DebugWidget();

	private:
		class DebugWidgetInternal;
		DebugWidgetInternal* mInternal;
	};
}



