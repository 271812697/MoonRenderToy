#pragma once
#include "Widgets/Property.h"
namespace MOON {
	class SliderFloatProperty :public WidgetProperty {
	public:
		SliderFloatProperty(const QString& n, PropertyComponent* comp);
		SliderFloatProperty(const QString& n, PropertyComponent* comp,float a,float b);
		~SliderFloatProperty();
		void setMinMax(float a,float b);
		void setStep(float step);
		virtual PropertyQtWidget* createEditorWidget(QWidget* parent = nullptr)override;
	private:
		float minA = -10.f;
		float maxB = 10.0f;
		float step = 0.02f;
	};

}