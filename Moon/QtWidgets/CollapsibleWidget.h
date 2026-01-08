#pragma once
#include <QWidget>
class QVBoxLayout;
namespace MOON {
    class CollapsibleWidget : public QWidget
    {
        Q_OBJECT
    public:
        explicit CollapsibleWidget(const QString& title, QWidget* parent = nullptr);
        ~CollapsibleWidget();
        // 获取内容区域的布局，用于添加属性控件
        QVBoxLayout* contentLayout();


    protected:
        // 让标题栏支持点击
        void mousePressEvent(QMouseEvent* event) override;

    private:
        class CollapsibleWidgetInternal;
        CollapsibleWidgetInternal* mInternal;

    };
}

