#pragma once
#include<vector>
#include <QString>
#include <QVariant>
namespace Core::ECS::Components
{
	class AComponent;
}
namespace MOON {
	class WidgetProperty;
	class PropertyComponent
	{
	public:
		PropertyComponent();
		virtual ~PropertyComponent();
		std::vector<WidgetProperty*>getProperties();
		
		virtual QVariant getPropertyValue(const QString& propertyName);
		virtual void setPropertyValue(const QString& propertyName, const QVariant& value);
		void updateWidgetValue();
		virtual QString getComponentName();
	protected:
		std::vector<WidgetProperty*>mProperties;
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