#pragma once
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QScrollArea>
#include "BaseTaskDialog.h"
namespace MOON {
    class TaskViewWidget : public QWidget
    {
        Q_OBJECT
    public:
        explicit TaskViewWidget(QWidget* parent = nullptr);

        // 切换任务UI
        void setTaskDialog(BaseTaskDialog* dlg);
        // 清空任务
        void clearTask();

    signals:
        void taskOk();
        void taskApply();
        void taskCancel();

    private:
        QVBoxLayout* m_mainLayout;
        QWidget* m_contentWidget;
        QVBoxLayout* m_contentLayout;
        BaseTaskDialog* m_currentTask = nullptr;
    };
}

