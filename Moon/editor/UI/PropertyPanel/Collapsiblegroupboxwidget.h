#pragma once
#include <QWidget>
#include "editor/UI/PropertyPanel/Property.h"
namespace MOON {
	class CollapsibleGroupBoxWidget:public QWidget {
		Q_OBJECT
	public:
		CollapsibleGroupBoxWidget(const QString& name,QWidget* parent);
		~CollapsibleGroupBoxWidget();
		void addProperty(Property* tmpProperty);
		void insertProperty(Property* prop, size_t index);
		void insertPropertyWidget(QWidget* propertyWidget, bool insertAtEnd);
		virtual QSize sizeHint() const override;
		virtual QSize minimumSizeHint() const override;
		void setCollapsed(bool collapse);
	private:
		class CollapsibleGroupBoxWidgetInternal;
		CollapsibleGroupBoxWidgetInternal* mInternal = nullptr;
	};
}