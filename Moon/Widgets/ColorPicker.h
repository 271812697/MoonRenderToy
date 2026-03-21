#pragma once
#include "Widgets/PropertyQtWidgets.h"
#include <QColor>
#include <QColorDialog> 
class QLabel;
class QLineEdit;
class QPushButton;
class QHBoxLayout;
namespace MOON {

    class ColorPicker : public PropertyQtWidget
    {
        Q_OBJECT

    public:
        explicit ColorPicker(QWidget* parent, WidgetProperty* prop);

        // 获取当前选中的颜色
        QColor currentColor() const;
        // 设置初始颜色
        void setCurrentColor(const QColor& color);
        virtual QVariant widgetValue()override;
        virtual void setWidgetValue(const QVariant& value) override;
    private slots:
        // 打开颜色选择对话框
        void openColorDialog();
        // 从输入框的文本更新颜色
        void updateColorFromText();

    private:
        // 更新UI显示（颜色预览和文本框）
        void updateUI(const QColor& color);

        QLabel* m_colorPreview;    // 颜色预览标签
        QLineEdit* m_colorEdit;    // 颜色值输入框
        QPushButton* m_pickButton; // 选择颜色按钮
        QColor m_selectedColor;    // 当前选中的颜色
    };


}