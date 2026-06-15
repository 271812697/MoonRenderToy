#pragma once
#include <QVariant>
#include <QString>
class QWidget;
namespace MOON {
	class PropertyQtWidget;
	class PropertyComponent;
	class WidgetProperty {
	public:
		WidgetProperty(const QString& n, PropertyComponent* comp);
		virtual ~WidgetProperty();
		virtual PropertyQtWidget* createEditorWidget(QWidget*parent=nullptr)=0;
		void updateWidgetValue(const QVariant& value);
		void setOwnerPropertyValue(const QVariant& value);
		void onWidgetValueChange();
		QString getPropertyName();
		QString getOwnerName();
	protected:
		PropertyComponent* owner = nullptr;
		PropertyQtWidget* mWidget = nullptr;
		QString mName;
	};
}