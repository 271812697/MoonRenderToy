#pragma once
#include "editor/UI/PropertyPanel/PropertyDef.h"
namespace Core::ECS::Components
{
	class AComponent;
}
namespace MOON {
	class Property;
	class ActorPropertyComponent 
	{
	public:
		ActorPropertyComponent(Core::ECS::Components::AComponent* comp) ;
		virtual ~ActorPropertyComponent() ;
		std::vector<Property*>getProperties();
		virtual Component componentData() ;
		virtual QVariant getPropertyValue(const QString& propertyName);
		virtual void setPropertyValue(const QString& propertyName, const QVariant& value);
		void updateWidgetValue();
		QString getComponentName();
	protected:
		Core::ECS::Components::AComponent * component=nullptr;
		std::vector<Property*>mProperties;
	};
}