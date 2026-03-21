#pragma once
#include "Widgets/Property.h"
#include "Widgets/sliderwidget.h"
namespace MOON {
	class SliderIntProperty :public WidgetProperty {
	public:
		SliderIntProperty(const QString& n, PropertyComponent* comp);
		~SliderIntProperty();
		virtual PropertyQtWidget* createEditorWidget(QWidget* parent = nullptr)override;
	};

}