#include "Widgets/SliderFloatProperty.h"
#include "Widgets/PropertyComponent.h"
namespace MOON {
	SliderFloatProperty::SliderFloatProperty(const QString& n, PropertyComponent* comp) :Property(n, comp) {

	}
	SliderFloatProperty::SliderFloatProperty(const QString& n, PropertyComponent* comp, float a, float b):Property(n, comp),minA(a),maxB(b)
	{
	}
	SliderFloatProperty::~SliderFloatProperty() {

	}
	void SliderFloatProperty::setMinMax(float a, float b)
	{
		if (widget) {
			widget->setMinValue(a);
			widget->setMaxValue(b);
		}
	}
	PropertyQtWidget* SliderFloatProperty::createEditorWidget(QWidget* parent) {
		if (widget == nullptr) {
			widget = new FloatSliderWidgetQt(parent);
			widget->setProp(this);
			widget->setValue(owner->getPropertyValue(mName).toFloat());
			widget->setMinValue(minA);
			widget->setMaxValue(maxB);
			widget->setIncrement(0.05f);
		}
		return widget;
	}
	void SliderFloatProperty::setOwnerPropertyValue(const QVariant& value) {
		owner->setPropertyValue(mName, value);
	}
	void SliderFloatProperty::onWidgetValueChange(){
		setOwnerPropertyValue(QVariant::fromValue(widget->getValue()));
	}
	void SliderFloatProperty::updateWidgetValue(const QVariant& value){
		//widget->setVec3Value(value.value<Maths::FVector3>());
	}
}