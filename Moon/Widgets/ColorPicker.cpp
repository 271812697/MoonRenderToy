#include "Widgets/ColorPicker.h"
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QHBoxLayout>
#include <QRegExpValidator>
#include <QPalette>
namespace MOON {
    ColorPicker::ColorPicker(QWidget* parent, WidgetProperty* prop)
        :PropertyQtWidget(parent, prop)
        , m_selectedColor(Qt::white) // 默认白色
    {
        // 1. 创建UI组件
        m_colorPreview = new QLabel(this);
       // m_colorPreview->setFixedSize(40, 25); // 预览框大小
        m_colorPreview->setStyleSheet("border: 1px solid #ccc;");

        m_colorEdit = new QLineEdit(this);
        m_colorEdit->setFixedWidth(100);
        // 限制输入为十六进制颜色值（#开头，6位十六进制）
        QRegExp colorRegex("#[0-9A-Fa-f]{6}");
        m_colorEdit->setValidator(new QRegExpValidator(colorRegex, this));

        m_pickButton = new QPushButton("picker color", this);

        // 2. 布局
        QHBoxLayout* mainLayout = new QHBoxLayout(this);
        mainLayout->addWidget(m_colorPreview);
        mainLayout->addWidget(m_colorEdit);
        mainLayout->addWidget(m_pickButton);
        mainLayout->setContentsMargins(5, 5, 5, 5);
        setLayout(mainLayout);

        // 3. 连接信号槽
        connect(m_pickButton, &QPushButton::clicked, this, &ColorPicker::openColorDialog);
        connect(m_colorEdit, &QLineEdit::editingFinished, this, &ColorPicker::updateColorFromText);

        // 4. 初始化UI
        //updateUI(m_selectedColor);
    }

    QColor ColorPicker::currentColor() const
    {
        return m_selectedColor;
    }
    QVariant ColorPicker::widgetValue() {
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
        // 打开系统颜色选择对话框，初始颜色为当前选中的颜色
        QColor selectedColor = QColorDialog::getColor(
            m_selectedColor,
            this,
            "pick color",
            QColorDialog::ShowAlphaChannel // 可选：显示透明度通道
        );

        // 如果用户选择了有效颜色，更新
        if (selectedColor.isValid()) {
            m_selectedColor = selectedColor;
            updateUI(selectedColor);
        }
    }

    void ColorPicker::updateColorFromText()
    {
        // 从输入框文本解析颜色
        QColor color(m_colorEdit->text());
        if (color.isValid()) {
            m_selectedColor = color;
            updateUI(color);
        }
        else {
            // 输入无效时，恢复原来的颜色值
            updateUI(m_selectedColor);
        }
        
    }

    void ColorPicker::updateUI(const QColor& color)
    {
        // 更新预览框颜色
        m_colorPreview->setStyleSheet(QString("background-color: %1; border: 1px solid #ccc;")
            .arg(color.name()));
        // 更新输入框文本（十六进制格式）
        m_colorEdit->setText(color.name());
        OnValueChanged();
    }
}