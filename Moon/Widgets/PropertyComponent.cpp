#pragma once
#include "Widgets/PropertyComponent.h"
#include "Widgets/Property.h"
#include "core/ECS/Components/AComponent.h"
namespace MOON {
	PropertyComponent::PropertyComponent()
	{
	}
	PropertyComponent::~PropertyComponent()
	{	
		for (auto prop : mProperties) {
			delete prop;
		}
	}
	std::vector<WidgetProperty*> PropertyComponent::getProperties()
	{
		return mProperties;
	}

	QVariant PropertyComponent::getPropertyValue(const QString& propertyName) {
		return QVariant();
	}
	void PropertyComponent::setPropertyValue(const QString& propertyName, const QVariant& value) {

	}
	void PropertyComponent::updateWidgetValue()
	{
		for (auto prop : mProperties) {
			prop->updateWidgetValue(getPropertyValue(prop->getPropertyName()));
		}
	}
	QString PropertyComponent::getComponentName()
	{
		return QString::fromStdString("Empty");
	}
	ActorPropertyComponent::ActorPropertyComponent(Core::ECS::Components::AComponent* comp):component(comp)
	{
	}
	ActorPropertyComponent::~ActorPropertyComponent()
	{
	}
	QString ActorPropertyComponent::getComponentName()
	{
		return QString::fromStdString(component->GetName());
	}
}