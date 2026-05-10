#pragma once
#include <QWidget>
#include <QVBoxLayout>
namespace MOON {
    class BaseTaskDialog : public QWidget
    {
        Q_OBJECT
    public:
        explicit BaseTaskDialog(QWidget* parent = nullptr);
        // 留给子类重写：构建界面
        virtual void buildUi() = 0;
        virtual void clickOk()=0;
        virtual void clickApply()=0;
        virtual void clickCancel()=0;

    protected:
        QVBoxLayout* m_layout;
    };
}
