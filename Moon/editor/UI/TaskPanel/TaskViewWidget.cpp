#include "editor/UI/TaskPanel/TaskViewWidget.h"
#include "Core/Global/ServiceLocator.h"
namespace MOON {
    TaskViewWidget::TaskViewWidget(QWidget* parent)
        : QWidget(parent)
    {
        //setMinimumWidth(280);
        //setStyleSheet("background-color:#f0f0f0;");
		RegService(TaskViewWidget, *this);
        m_mainLayout = new QVBoxLayout(this);
        m_mainLayout->setContentsMargins(0, 0, 0, 0);
        m_mainLayout->setSpacing(0);

        // 滚动内容区
        QScrollArea* scrollArea = new QScrollArea;
        scrollArea->setWidgetResizable(true);
        scrollArea->setFrameShape(QFrame::NoFrame);

        m_contentWidget = new QWidget;
        m_contentLayout = new QVBoxLayout(m_contentWidget);
        m_contentLayout->setAlignment(Qt::AlignTop);
        scrollArea->setWidget(m_contentWidget);

        m_mainLayout->addWidget(scrollArea);

        // 底部按钮 仿FreeCAD
        QHBoxLayout* btnLay = new QHBoxLayout;
        QPushButton* btnOk = new QPushButton("OK");
        QPushButton* btnApply = new QPushButton("Apply");
        QPushButton* btnCancel = new QPushButton("Cancel");

        btnLay->addStretch();
        btnLay->addWidget(btnOk);
        btnLay->addWidget(btnApply);
        btnLay->addWidget(btnCancel);

        m_mainLayout->addLayout(btnLay);

        connect(btnOk, &QPushButton::clicked, this, &TaskViewWidget::clickOk);
        connect(btnApply, &QPushButton::clicked, this, &TaskViewWidget::clickApply);
        connect(btnCancel, &QPushButton::clicked, this, &TaskViewWidget::clickCancel);
    }

    void TaskViewWidget::setTaskDialog(BaseTaskDialog* dlg)
    {
        if (!dlg) return;
        // 清空旧任务
        clearTask();
        // 设置新任务
        m_currentTask = dlg;
        m_contentLayout->addWidget(m_currentTask);
    }

    void TaskViewWidget::clearTask()
    {
        if (m_currentTask)
        {
            m_contentLayout->removeWidget(m_currentTask);
            m_currentTask->deleteLater();
            m_currentTask = nullptr;
        }
    }
    bool TaskViewWidget::hasTask()
    {
        return m_currentTask != nullptr;
    }
    void TaskViewWidget::clickCancel()
    {
        emit taskCancel();
        if (m_currentTask)
        {
            m_currentTask->clickCancel();
        }
    }
    void TaskViewWidget::clickOk()
    {
        emit taskOk();
        if (m_currentTask)
        {
            m_currentTask->clickOk();
        }
        clearTask();
    }
    void TaskViewWidget::clickApply()
    {
        emit taskApply();
        if (m_currentTask)
        {
            m_currentTask->clickApply();
        }
    }
}