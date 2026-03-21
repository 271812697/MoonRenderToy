#include "Widgets/SliderIntProperty.h"
#include "Widgets/PropertyComponent.h"
namespace MOON {
	SliderIntProperty::SliderIntProperty(const QString& n, PropertyComponent* comp) :WidgetProperty(n, comp) {

	}
	SliderIntProperty::~SliderIntProperty() {

	}
	PropertyQtWidget* SliderIntProperty::createEditorWidget(QWidget* parent ) {
		if (mWidget == nullptr) {
			auto widget = new IntSliderWidgetQt(parent);
			mWidget = widget;
			widget->setProp(this);
			widget->setValue(owner->getPropertyValue(mName).toInt());
			widget->setMinValue(-10);
			widget->setMaxValue(10);
			widget->setIncrement(1);
		}
		return mWidget;
	}
}