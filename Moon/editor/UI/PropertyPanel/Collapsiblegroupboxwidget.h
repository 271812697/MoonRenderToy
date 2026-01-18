#pragma once
#include <QWidget>
#include "Widgets/Property.h"
namespace MOON {
	class CollapsibleGroupBoxWidget:public QWidget {
		Q_OBJECT
	public:
		CollapsibleGroupBoxWidget(const QString& name,QWidget* parent);
		~CollapsibleGroupBoxWidget();
		void addProperty(Property* tmpProperty);
		void insertProperty(Property* prop, size_t index);
		void insertPropertyWidget(const QString& label, PropertyQtWidget* propertyWidget, bool insertAtEnd);
		virtual QSize sizeHint() const override;
		virtual QSize minimumSizeHint() const override;
		void setCollapsed(bool collapse);
	private:
		class CollapsibleGroupBoxWidgetInternal;
		CollapsibleGroupBoxWidgetInternal* mInternal = nullptr;
	};
}