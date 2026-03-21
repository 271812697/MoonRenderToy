#include "Widgets/SlideChekBox.h"
#include <QCheckBox>
#include<QHBoxLayout>
namespace MOON {
	SliderCheckBox::SliderCheckBox(QWidget* parent, WidgetProperty* prop):PropertyQtWidget(parent,prop)
	{
		m_checkBox = new SlidingCheckBox(this);
        QHBoxLayout* hLayout = new QHBoxLayout();
		connect(m_checkBox, &QAbstractButton::toggled, this, &SliderCheckBox::toggle);
        
        hLayout->addWidget(m_checkBox);
       
        hLayout->setContentsMargins(0, 0, 0, 0);
		setLayout(hLayout);

	}
	bool SliderCheckBox::getValue() const
	{
		return m_checkBox->isChecked();
	}
	void SliderCheckBox::setValue(bool val)
	{
		m_checkBox->setCheckValue(val);
	}
	QVariant SliderCheckBox::widgetValue()
	{
		return getValue();;
	}
	void SliderCheckBox::setWidgetValue(const QVariant& value)
	{
		setValue(value.toBool());
	}
	void SliderCheckBox::toggle(bool val)
	{
		OnValueChanged();
	}
}