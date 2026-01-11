#pragma once
#include "editor/UI/PropertyPanel/PropertyComponent.h"
#include "editor/UI/PropertyPanel/Property.h"
#include "core/ECS/Components/AComponent.h"
namespace MOON {
	ActorPropertyComponent::ActorPropertyComponent(Core::ECS::Components::AComponent* comp):component(comp)
	{
	}
	ActorPropertyComponent::~ActorPropertyComponent()
	{
		for (auto prop : mProperties) {
			delete prop;
		}
	}
	std::vector<Property*> ActorPropertyComponent::getProperties()
	{
		return mProperties;
	}
	Component ActorPropertyComponent::componentData() {
		Component res;
		res.name = QString::fromStdString(component->GetName());
		return res;
	}
	QVariant ActorPropertyComponent::getPropertyValue(const QString& propertyName) {
		return QVariant();
	}
	void ActorPropertyComponent::setPropertyValue(const QString& propertyName, const QVariant& value) {

	}
	void ActorPropertyComponent::updateWidgetValue()
	{
		for (auto prop : mProperties) {
			prop->updateWidgetValue(getPropertyValue(prop->getPropertyName()));
		}
	}
	QString ActorPropertyComponent::getComponentName()
	{
		return QString::fromStdString(component->GetName());
	}
}