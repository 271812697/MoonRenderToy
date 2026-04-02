#pragma once
#include <QWidget>
namespace MOON {
	class RenderPassSettingWidget : public QWidget
	{
	public:
		RenderPassSettingWidget(QWidget* parent);
		~RenderPassSettingWidget();
		void Refresh();
	private:
		class RenderPassSettingWidgetInternal;
		RenderPassSettingWidgetInternal* mInternal=nullptr;
	};
}