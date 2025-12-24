#pragma once
#include <QWidget>
#include <QTreeWidgetItem>
namespace MOON {
	class SettingWidget : public QWidget
	{
	public:
		SettingWidget(QWidget* parent);
		~SettingWidget();
    public slots:
		void onItemChanged(QTreeWidgetItem* current, QTreeWidgetItem* previous);
	private:
		class SettingWidgetInternal;
		SettingWidgetInternal* mInternal=nullptr;
	};
}