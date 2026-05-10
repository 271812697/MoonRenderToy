#include "SketchTaskDialog.h"
#include "TaskBox.h"
#include "Sketcher/SketcherObjManager.h"
#include "Sketcher/SketcherObj.h"
#include <QLabel>
#include <QLineEdit>
#include <QCheckBox>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QGroupBox>
namespace MOON {
    SketchTaskDialog::SketchTaskDialog(QWidget* parent)
        : BaseTaskDialog(parent)
    {
        buildUi();
        SketcherObjManager::instance().Push();
        sketchObj = SketcherObjManager::instance().GetCurrentActiveSketcherObj();
    }

    void SketchTaskDialog::buildUi()
    {
        // ========== 1. 草图常规设置 ==========
        TaskBox* boxGeneral = new TaskBox(QStringLiteral("草图常规设置"));
        QWidget* wGeneral = new QWidget;
        QVBoxLayout* layGeneral = new QVBoxLayout(wGeneral);
        layGeneral->setSpacing(6);

        layGeneral->addWidget(new QLabel(QStringLiteral("草图名称")));
        QLineEdit* edtName = new QLineEdit(QStringLiteral("Sketch001"));
        layGeneral->addWidget(edtName);

        layGeneral->addWidget(new QLabel(QStringLiteral("附着基准平面")));
        QComboBox* cboDatum = new QComboBox;
        cboDatum->addItems({
            QStringLiteral("XY平面"),
            QStringLiteral("XZ平面"),
            QStringLiteral("YZ平面"),
            QStringLiteral("自定义基准面")
            });
        layGeneral->addWidget(cboDatum);
        boxGeneral->setContent(wGeneral);
        mainLayout()->addWidget(boxGeneral);

        // ========== 2. 自动约束设置 ==========
        TaskBox* boxConstraint = new TaskBox(QStringLiteral("自动约束设置"));
        QWidget* wConstraint = new QWidget;
        QVBoxLayout* layConstraint = new QVBoxLayout(wConstraint);

        layConstraint->addWidget(new QCheckBox(QStringLiteral("启用自动约束")));
        layConstraint->addWidget(new QCheckBox(QStringLiteral("重合约束")));
        layConstraint->addWidget(new QCheckBox(QStringLiteral("水平/竖直约束")));
        layConstraint->addWidget(new QCheckBox(QStringLiteral("平行/垂直约束")));
        boxConstraint->setContent(wConstraint);
        mainLayout()->addWidget(boxConstraint);

        // ========== 3. 显示选项 ==========
        TaskBox* boxView = new TaskBox(QStringLiteral("显示选项"));
        QWidget* wView = new QWidget;
        QVBoxLayout* layView = new QVBoxLayout(wView);

        layView->addWidget(new QCheckBox(QStringLiteral("显示网格")));
        layView->addWidget(new QCheckBox(QStringLiteral("显示坐标轴")));
        layView->addWidget(new QCheckBox(QStringLiteral("隐藏背景零件")));
        boxView->setContent(wView);
        mainLayout()->addWidget(boxView);

        mainLayout()->addStretch();
    }
}