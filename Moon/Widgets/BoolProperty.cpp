#include "Widgets/BoolProperty.h"
#include "Widgets/PropertyComponent.h"
namespace MOON {
	BoolProperty::BoolProperty(const QString& n, PropertyComponent* comp) :Property(n, comp) {

	}
	BoolProperty::~BoolProperty() {

	}
	PropertyQtWidget* BoolProperty::createEditorWidget(QWidget* parent ) {
		if (widget == nullptr) {
			widget = new SliderCheckBox(parent, this);
			widget->setValue(owner->getPropertyValue(mName).value<bool>());
		}
		return widget;
	}
	void BoolProperty::setOwnerPropertyValue(const QVariant& value) {
		owner->setPropertyValue(mName, value);
	}
	void BoolProperty::onWidgetValueChange(){
		setOwnerPropertyValue(QVariant::fromValue(widget->getValue()));
	}
	void BoolProperty::updateWidgetValue(const QVariant& value) {
	}
}