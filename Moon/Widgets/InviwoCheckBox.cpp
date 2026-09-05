#include "Widgets/InviwoCheckBox.h"
#include <QHBoxLayout>
#include <QPainter>

namespace MOON {

	RectCheckBoxButton::RectCheckBoxButton(QWidget* parent)
		: QAbstractButton(parent)
	{
		setCheckable(true);
		setCursor(Qt::PointingHandCursor);
	}

	void RectCheckBoxButton::paintEvent(QPaintEvent* /*event*/)
	{
		QPainter p(this);
		p.setRenderHint(QPainter::Antialiasing, true);

		const QRectF box = QRectF(rect()).adjusted(1.5, 1.5, -1.5, -1.5);
		QColor fill;
		if (isChecked()) {
			fill = QColor("#4f9cf6");  // highlighted when selected
		}
		else {
			fill = QColor("#8f8f98");  // brighter grey, distinct from background
		}
		if (underMouse()) {
			fill = fill.lighter(112);
		}

		p.setPen(Qt::NoPen);
		p.setBrush(fill);
		p.drawRect(box);  // sharp (right-angle) rectangle, no rounded corners
	}

	InviwoCheckBox::InviwoCheckBox(QWidget* parent, WidgetProperty* prop)
		: PropertyQtWidget(parent, prop)
	{
		m_box = new RectCheckBoxButton(this);
		connect(m_box, &QAbstractButton::toggled, this, &InviwoCheckBox::toggle);

		auto* layout = new QHBoxLayout(this);
		layout->setContentsMargins(0, 0, 0, 0);
		layout->addWidget(m_box, 0, Qt::AlignRight);
	}

	bool InviwoCheckBox::getValue() const
	{
		return m_box->isChecked();
	}

	void InviwoCheckBox::setValue(bool val)
	{
		m_box->setChecked(val);
	}

	QVariant InviwoCheckBox::widgetValue()
	{
		return getValue();
	}

	void InviwoCheckBox::setWidgetValue(const QVariant& value)
	{
		setValue(value.toBool());
	}

	void InviwoCheckBox::toggle(bool /*checked*/)
	{
		OnValueChanged();
	}

}
