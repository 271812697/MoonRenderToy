#include "Widgets/FVec3Property.h"
#include "Widgets/PropertyComponent.h"
namespace MOON {
	FVec3Property::FVec3Property(const QString& n, PropertyComponent* comp) :Property(n, comp) {

	}
	FVec3Property::~FVec3Property() {

	}
	PropertyQtWidget* FVec3Property::createEditorWidget(QWidget* parent ){
		if (widget == nullptr) {
			widget = new Fvec3(parent, this);
			widget->setVec3Value(owner->getPropertyValue(mName).value<Maths::FVector3>());
		}
		return widget;
	}
	void FVec3Property::setOwnerPropertyValue(const QVariant& value) {
		owner->setPropertyValue(mName, value);
	}
	void FVec3Property::onWidgetValueChange() {
		setOwnerPropertyValue(QVariant::fromValue(widget->getVec3Value()));
	}
	void FVec3Property::updateWidgetValue(const QVariant& value){
		widget->setVec3Value(value.value<Maths::FVector3>());
	}
}