#pragma once
#include <QVariant>
#include <QString>
class QWidget;
namespace MOON {
	class PropertyQtWidget;
	class PropertyComponent;
	class Property {
	public:
		Property(const QString& n, PropertyComponent* comp);
		virtual ~Property();
		virtual PropertyQtWidget* createEditorWidget(QWidget*parent=nullptr)=0;
		virtual void setOwnerPropertyValue(const QVariant& value)=0;
		virtual void updateWidgetValue(const QVariant& value) = 0;
		virtual void onWidgetValueChange()=0;
		QString getPropertyName();
	protected:
		PropertyComponent* owner = nullptr;
		QString mName;
	};
}