#pragma once
#include "Widgets/PropertyQtWidgets.h"
#include <QColor>
class QLineEdit;
class QColorDialog;
class QToolButton;
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

        QToolButton* m_colorButton = nullptr;  // Inviwo style colour swatch
        QLineEdit* m_colorEdit = nullptr;      // hexadecimal value editor
        QColorDialog* m_colorDialog = nullptr; // non-modal, live update dialog
        QColor m_selectedColor;                // 当前选中的颜色
    };


}
