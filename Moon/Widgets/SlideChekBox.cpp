#include "Widgets/SlideChekBox.h"
#include <QCheckBox>
#include<QHBoxLayout>
namespace MOON {
	SliderCheckBox::SliderCheckBox(QWidget* parent, Property* prop):PropertyQtWidget(parent,prop)
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
		m_checkBox->setChecked(val);
	}
	void SliderCheckBox::toggle(bool val)
	{
		OnValueChanged();
	}
}