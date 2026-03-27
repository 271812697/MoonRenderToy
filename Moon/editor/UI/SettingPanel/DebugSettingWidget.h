#pragma once
#include <QWidget>
namespace MOON {
	class DebugSettingWidget : public QWidget
	{
	public:
		DebugSettingWidget(QWidget* parent);
		~DebugSettingWidget();
		void Refresh();
	private:
		class DebugSettingWidgetInternal;
		DebugSettingWidgetInternal* mInternal=nullptr;
	};
}