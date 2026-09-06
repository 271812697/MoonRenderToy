#pragma once
#include "Widgets/PropertyQtWidgets.h"
#include <QAbstractButton>

namespace MOON {

	// Inviwo style boolean editor: a small square that is filled with the
	// highlight colour when checked and stays grey when unchecked.
	class RectCheckBoxButton : public QAbstractButton
	{
		Q_OBJECT
	public:
		explicit RectCheckBoxButton(QWidget* parent = nullptr);
		QSize sizeHint() const override
		{
			return QSize(24, 24);
		}
		QSize minimumSizeHint() const override
		{
			return QSize(24, 24);
		}
	protected:
		void paintEvent(QPaintEvent* event) override;
	};

	class InviwoCheckBox : public PropertyQtWidget
	{
		Q_OBJECT
	public:
		InviwoCheckBox(QWidget* parent, WidgetProperty* prop);
		bool getValue() const;
		void setValue(bool val);
		virtual QVariant widgetValue() override;
		virtual void setWidgetValue(const QVariant& value) override;
	public Q_SLOTS:
		void toggle(bool checked);
	private:
		RectCheckBoxButton* m_box = nullptr;
	};

}
