#pragma once
#include <QVariant>
#include <QString>
class QWidget;
namespace MOON {
	class ActorPropertyComponent;
	class Property {
	public:
		Property(const QString& n,ActorPropertyComponent* comp);
		virtual ~Property();
		virtual QWidget* createEditorWidget(QWidget*parent=nullptr)=0;
		virtual void setPropertyValue(const QVariant& value)=0;
		virtual void updateWidgetValue(const QVariant& value) = 0;
		virtual void onPropertyValueChange()=0;
		QString getPropertyName();
	protected:
		ActorPropertyComponent* owner = nullptr;
		QString mName;
	};
}