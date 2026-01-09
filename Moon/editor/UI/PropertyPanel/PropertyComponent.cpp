#pragma once
#include "editor/UI/PropertyPanel/PropertyComponent.h"
#include "core/ECS/Components/AComponent.h"
namespace MOON {
	ActorPropertyComponent::ActorPropertyComponent(Core::ECS::Components::AComponent* comp):component(comp)
	{
	}
	ActorPropertyComponent::~ActorPropertyComponent()
	{
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
}