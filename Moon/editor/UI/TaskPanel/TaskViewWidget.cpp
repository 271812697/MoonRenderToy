#include "editor/UI/TaskPanel/TaskViewWidget.h"
#include "Core/Global/ServiceLocator.h"
#include "editor/UI/TaskPanel/ExtrudeTaskDialog.h"
#include "editor/UI/TaskPanel/SketchTaskDialog.h"
#include "editor/UI/TaskPanel/ThicknessTaskDialog.h"
#include "editor/UI/TaskPanel/FilletTask.h"
#include "editor/UI/TaskPanel/RevolutionTask.h"
#include "TopoShape.h"
#include "feature/SketcherFeature.h"
#include "feature/ExtrudeFeature.h"
#include "feature/ThicknessFeature.h"
#include "feature/FilletFeature.h"
#include "feature/RevolveFeature.h"


namespace MOON {
    BaseTaskDialog* createFeatureDialog(Feature* feature) {
        ExtrudeFeature* extrude=dynamic_cast<ExtrudeFeature*>(feature);
        if (extrude) {
            ExtrudeTaskDialog* dialog = new ExtrudeTaskDialog(nullptr, extrude->addSubType == 0 ? ExtrudeType::Additive : ExtrudeType::Subtractive,feature);
            return dialog;
        }

        SketcherFeature* sketcher = dynamic_cast<SketcherFeature*>(feature);
        if (sketcher) {
			SketchTaskDialog* dialog = new SketchTaskDialog(nullptr,sketcher);
            return dialog;
        }

		ThicknessFeature* thickness = dynamic_cast<ThicknessFeature*>(feature);
        if (thickness) {
			ThicknessTaskDialog* dialog = new ThicknessTaskDialog(nullptr, thickness);
			return dialog;
        }
        FilletFeature* fillet = dynamic_cast<FilletFeature*>(feature);
        if (fillet) {
            FilletTask* dialog = new FilletTask(nullptr,fillet);
            return dialog;
        }
        RevolveFeature* revolve = dynamic_cast<RevolveFeature*>(feature);
        if (revolve) {
            RevolutionTask* dialog = new RevolutionTask(revolve->addSubType==0? RevolutionType::ReAdditive: RevolutionType::ReSubtractive,nullptr,revolve);
            return dialog;
        }
        return nullptr;
    }
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
        clearTask();
    }
    void TaskViewWidget::onSelectedFeature(void* feature)
    {
		if (!hasTask())
		{
            
            setTaskDialog(createFeatureDialog( static_cast<Feature*>(feature)));
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