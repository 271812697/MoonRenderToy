#pragma once
#include "Widgets/Property.h"
#include "Widgets/PropertyQtWidgets.h"
#include "Widgets/PropertyComponent.h"
namespace MOON {
	WidgetProperty::WidgetProperty(const QString& n, PropertyComponent* comp):mName(n), owner(comp)
	{
	}
	WidgetProperty::~WidgetProperty() {
	
	}
	QString WidgetProperty::getPropertyName()
	{
		return mName;
	}
	QString WidgetProperty::getOwnerName()
	{
		return owner->getComponentName();
	}
	void WidgetProperty::setOwnerPropertyValue(const QVariant& value) {
		owner->setPropertyValue(mName,value);
	}
	void WidgetProperty::onWidgetValueChange() {
		setOwnerPropertyValue(mWidget->widgetValue());
	}
	void WidgetProperty::updateWidgetValue(const QVariant& value) {
		mWidget->setWidgetValue(value);
	}
}