#pragma once
#include <QWidget>
namespace MOON {
	class RenderSettingWidget : public QWidget
	{
	public:
		RenderSettingWidget(QWidget* parent);
		~RenderSettingWidget();
		void Refresh();
	private:
		class RenderSettingWidgetInternal;
		RenderSettingWidgetInternal* mInternal=nullptr;
	};
}