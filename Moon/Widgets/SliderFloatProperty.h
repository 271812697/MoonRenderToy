#pragma once
#include "Widgets/Property.h"
#include "Widgets/sliderwidget.h"
namespace MOON {
	class SliderFloatProperty :public Property {
	public:
		SliderFloatProperty(const QString& n, PropertyComponent* comp);
		SliderFloatProperty(const QString& n, PropertyComponent* comp,float a,float b);
		~SliderFloatProperty();
		void setMinMax(float a,float b);
		virtual PropertyQtWidget* createEditorWidget(QWidget* parent = nullptr)override;
		virtual void setOwnerPropertyValue(const QVariant& value)override;
		virtual void onWidgetValueChange()override;
		virtual void updateWidgetValue(const QVariant& value)override;
	private:
		float minA = -10.f;
		float maxB = 10.0f;

		FloatSliderWidgetQt* widget = nullptr;
	};

}