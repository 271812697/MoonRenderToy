#pragma once
#include "editor/UI/PropertyPanel/PropertyDef.h"
namespace Core::ECS::Components
{
	class AComponent;
}
namespace MOON {
	class ActorPropertyComponent 
	{
	public:
		ActorPropertyComponent(Core::ECS::Components::AComponent* comp) ;
		virtual ~ActorPropertyComponent() ;
		virtual Component componentData() ;
		virtual QVariant getPropertyValue(const QString& propertyName);
		virtual void setPropertyValue(const QString& propertyName, const QVariant& value);

	private:
		Core::ECS::Components::AComponent * component=nullptr;
	};
}