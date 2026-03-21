#include "Widgets/ComboBox.h"
#include<QHBoxLayout>
namespace MOON {
	ComboBox::ComboBox(QWidget* parent, WidgetProperty* prop):PropertyQtWidget(parent,prop)
	{
		myCbox = new QComboBox(this);
		QHBoxLayout* hLayout = new QHBoxLayout();
		

		hLayout->addWidget(myCbox);

		hLayout->setContentsMargins(0, 0, 0, 0);
		setLayout(hLayout);
		connect(myCbox, &QComboBox::currentTextChanged, this, &ComboBox::onProvinceSelect);
	}
	void ComboBox::addComboList(const QList<QString>& list)
	{
		for (int i = 0; i < list.size(); i++) {
			myCbox->addItem(list[i]);
		}
	}
	int ComboBox::getCurrentIndex()
	{
		return myCbox->currentIndex();
	}
	QVariant ComboBox::widgetValue() {
		return getCurrentIndex();
	}
	void ComboBox::setWidgetValue(const QVariant& value)
	{
		myCbox->setCurrentIndex(value.toInt());
	}
	void ComboBox::onProvinceSelect(const QString& selectText) {
	
		OnValueChanged();
	}
}