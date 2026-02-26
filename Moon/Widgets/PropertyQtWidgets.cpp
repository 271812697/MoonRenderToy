#include "Widgets/PropertyQtWidgets.h"
#include "Widgets/Property.h"
namespace MOON {
	PropertyQtWidget::PropertyQtWidget(QWidget* parent, Property* prop):QWidget(parent),mProps(prop)
	{
	}
	PropertyQtWidget::PropertyQtWidget(QWidget* parent):QWidget(parent)
	{
	}
	void PropertyQtWidget::setProp(Property* prop)
	{
		mProps = prop;
	}
	void PropertyQtWidget::OnValueChanged()
	{
		mProps->onWidgetValueChange();
	}

}