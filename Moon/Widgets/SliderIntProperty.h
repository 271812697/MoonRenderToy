#pragma once
#include "Widgets/Property.h"
#include "Widgets/sliderwidget.h"
namespace MOON {
	class SliderIntProperty :public Property {
	public:
		SliderIntProperty(const QString& n, PropertyComponent* comp);
		~SliderIntProperty();
		virtual PropertyQtWidget* createEditorWidget(QWidget* parent = nullptr)override;
		virtual void setOwnerPropertyValue(const QVariant& value)override;
		virtual void onWidgetValueChange()override;
		virtual void updateWidgetValue(const QVariant& value)override;
	private:
		IntSliderWidgetQt* widget = nullptr;
	};

}