#pragma once
#include "editor/UI/PropertyPanel/PropertyDef.h"
namespace Core::ECS::Components
{
	class AComponent;
}
namespace MOON {
	class Property;
	class PropertyComponent
	{
	public:
		PropertyComponent();
		virtual ~PropertyComponent();
		std::vector<Property*>getProperties();
		virtual Component componentData();
		virtual QVariant getPropertyValue(const QString& propertyName);
		virtual void setPropertyValue(const QString& propertyName, const QVariant& value);
		void updateWidgetValue();
		virtual QString getComponentName();
	protected:
		std::vector<Property*>mProperties;
	};
	class ActorPropertyComponent :public PropertyComponent
	{
	public:
		ActorPropertyComponent(Core::ECS::Components::AComponent* comp) ;
		virtual ~ActorPropertyComponent() ;
		virtual QString getComponentName()override;
	protected:
		Core::ECS::Components::AComponent * component=nullptr;
	};
}