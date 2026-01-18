#include "Widgets/EnumProperty.h"
#include "Widgets/PropertyComponent.h"
namespace MOON {
	EnumProperty::EnumProperty(const QString& n, PropertyComponent* comp) :Property(n, comp) {

	}
	EnumProperty::~EnumProperty() {

	}
	PropertyQtWidget* EnumProperty::createEditorWidget(QWidget* parent) {
		if (widget == nullptr) {
			widget = new ComboBox(parent, this);
			widget->addComboList(owner->getPropertyValue(mName).value<QList<QString>>());
		}
		return widget;
	}
	void EnumProperty::setOwnerPropertyValue(const QVariant& value) {
		owner->setPropertyValue(mName, value);
	}
	void EnumProperty::onWidgetValueChange(){
		setOwnerPropertyValue(QVariant::fromValue(widget->getCurrentIndex()));
	}
	void EnumProperty::updateWidgetValue(const QVariant& value){
	}
}