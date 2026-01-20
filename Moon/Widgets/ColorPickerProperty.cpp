#include "Widgets/ColorPickerProperty.h"
#include "Widgets/PropertyComponent.h"
namespace MOON {
	ColorPickerProperty::ColorPickerProperty(const QString& n, PropertyComponent* comp) :Property(n, comp) {

	}
	ColorPickerProperty::~ColorPickerProperty() {

	}
	PropertyQtWidget* ColorPickerProperty::createEditorWidget(QWidget* parent){
		if (widget == nullptr) {
			widget = new ColorPicker(parent, this);
			widget->setCurrentColor(owner->getPropertyValue(mName).value<QColor>());
		}
		return widget;
	}
	void ColorPickerProperty::setOwnerPropertyValue(const QVariant& value){
		owner->setPropertyValue(mName, value);
	}
	void ColorPickerProperty::onWidgetValueChange(){
		setOwnerPropertyValue(QVariant::fromValue(widget->currentColor()));
	}
	void ColorPickerProperty::updateWidgetValue(const QVariant& value) {
		//widget->setVec3Value(value.value<Maths::FVector3>());
	}
}