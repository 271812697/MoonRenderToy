#include "Widgets/ColorPicker.h"
#include <QColorDialog>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include <QToolButton>

namespace MOON {
	ColorPicker::ColorPicker(QWidget* parent, WidgetProperty* prop)
		: PropertyQtWidget(parent, prop)
		, m_selectedColor(Qt::white)
	{
		m_colorButton = new QToolButton(this);
		m_colorButton->setObjectName("ColorButton");
		m_colorButton->setAutoRaise(true);
		m_colorButton->setFixedSize(48, 32);
		m_colorButton->setCursor(Qt::PointingHandCursor);
		m_colorButton->setFocusPolicy(Qt::ClickFocus);

		m_colorEdit = new QLineEdit(this);
		m_colorEdit->setFixedWidth(100);
		// Accept #RRGGBB and #AARRGGBB.
		QRegularExpression colorRegex("#[0-9A-Fa-f]{6}([0-9A-Fa-f]{2})?");
		m_colorEdit->setValidator(
			new QRegularExpressionValidator(colorRegex, this)
		);
		m_colorEdit->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

		auto* mainLayout = new QHBoxLayout(this);
		mainLayout->setContentsMargins(0, 0, 0, 0);
		mainLayout->setSpacing(6);
		mainLayout->setAlignment(Qt::AlignLeft);
		mainLayout->addWidget(m_colorButton);
		mainLayout->addWidget(m_colorEdit, 1);

		connect(m_colorButton, &QToolButton::clicked, this, &ColorPicker::openColorDialog);
		connect(m_colorEdit, &QLineEdit::editingFinished, this, &ColorPicker::updateColorFromText);
	}

	QColor ColorPicker::currentColor() const
	{
		return m_selectedColor;
	}

	QVariant ColorPicker::widgetValue()
	{
		return currentColor();
	}

	void ColorPicker::setWidgetValue(const QVariant& value)
	{
		setCurrentColor(value.value<QColor>());
	}

	void ColorPicker::setCurrentColor(const QColor& color)
	{
		if (color.isValid()) {
			m_selectedColor = color;
			updateUI(color);
		}
	}

	void ColorPicker::openColorDialog()
	{
		if (!m_colorDialog) {
			m_colorDialog = new QColorDialog(this);
			m_colorDialog->setWindowModality(Qt::NonModal);
			m_colorDialog->setOption(QColorDialog::ShowAlphaChannel, true);
			m_colorDialog->setOption(QColorDialog::NoButtons, true);
			connect(
				m_colorDialog,
				&QColorDialog::currentColorChanged,
				this,
				[this](const QColor& color) {
					if (color.isValid()) {
						m_selectedColor = color;
						updateUI(color);
					}
				}
			);
		}
		m_colorDialog->setCurrentColor(m_selectedColor);
		m_colorDialog->show();
		m_colorDialog->raise();
		m_colorDialog->activateWindow();
	}

	void ColorPicker::updateColorFromText()
	{
		const QColor color(m_colorEdit->text());
		if (color.isValid()) {
			setCurrentColor(color);
		}
		else {
			updateUI(m_selectedColor);
		}
	}

	void ColorPicker::updateUI(const QColor& color)
	{
		// Inviwo style: swatch shows the current colour, hover adds an accent
		// border; the line edit carries the hexadecimal value.
		const QString css = QString(
			"QToolButton#ColorButton {"
			" border: 1px solid transparent;"
			" background-color: %1;"
			" border-radius: 5px;"
			"}"
			"QToolButton#ColorButton:hover,"
			"QToolButton#ColorButton:pressed {"
			" border: 1px solid #268bd2;"
			"}"
		).arg(color.name());
		m_colorButton->setStyleSheet(css);
		m_colorEdit->setText(
			color.alpha() < 255 ? color.name(QColor::HexArgb) : color.name()
		);
		OnValueChanged();
	}
}
