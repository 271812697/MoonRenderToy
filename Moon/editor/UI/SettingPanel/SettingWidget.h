#pragma once
#include <QWidget>
namespace MOON {
	class SettingWidget : public QWidget
	{
	public:
		SettingWidget(QWidget* parent);
		~SettingWidget();
	private:
		class SettingWidgetInternal;
		SettingWidgetInternal* mInternal = nullptr;
	};
}
