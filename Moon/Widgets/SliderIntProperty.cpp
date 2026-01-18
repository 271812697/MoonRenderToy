#include "Widgets/SliderIntProperty.h"
#include "Widgets/PropertyComponent.h"
namespace MOON {
	SliderIntProperty::SliderIntProperty(const QString& n, PropertyComponent* comp) :Property(n, comp) {

	}
	SliderIntProperty::~SliderIntProperty() {

	}
	PropertyQtWidget* SliderIntProperty::createEditorWidget(QWidget* parent ) {
		if (widget == nullptr) {
			widget = new IntSliderWidgetQt(parent);
			widget->setProp(this);
			widget->setValue(owner->getPropertyValue(mName).toInt());
			widget->setMinValue(-10);
			widget->setMaxValue(10);
			widget->setIncrement(1);
		}
		return widget;
	}
	void SliderIntProperty::setOwnerPropertyValue(const QVariant& value){
		owner->setPropertyValue(mName, value);
	}
	void SliderIntProperty::onWidgetValueChange(){
		setOwnerPropertyValue(QVariant::fromValue(widget->getValue()));
	}
	void SliderIntProperty::updateWidgetValue(const QVariant& value){
		//widget->setVec3Value(value.value<Maths::FVector3>());
	}
}