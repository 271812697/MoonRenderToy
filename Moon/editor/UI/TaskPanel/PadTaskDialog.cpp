#include "editor/UI/TaskPanel/PadTaskDialog.h"
#include "TaskBox.h"
#include "Sketcher/SketcherObjManager.h"
#include "Sketcher/SketcherObj.h"
#include "App/ExtrusionHelper.h"
#include "core/component/TopoShapeActor.h"
#include "core/component/CTopoShape.h"
#include "TopoShape.h"
#include "Core/Global/ServiceLocator.h"
#include "renderer/SceneView.h"
#include <QLabel>
#include <QLineEdit>
#include <QCheckBox>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QGroupBox>
#include <QRadioButton>
namespace MOON {
    PadTaskDialog::PadTaskDialog(QWidget* parent)
        : BaseTaskDialog(parent)
    {
        buildUi();
    }

    PadTaskDialog::~PadTaskDialog()
    {
    }

    void PadTaskDialog::buildUi()
    {
        // 1. 拉伸类型
        TaskBox* boxType = new TaskBox(QStringLiteral("拉伸类型"));
        QWidget* wType = new QWidget;
        QVBoxLayout* layType = new QVBoxLayout(wType);
        layType->setContentsMargins(0, 0, 0, 0);
        layType->setSpacing(6);

        QRadioButton* rbDim = new QRadioButton(QStringLiteral("给定长度"));
        QRadioButton* rbAll = new QRadioButton(QStringLiteral("贯穿全部"));
        QRadioButton* rbToFace = new QRadioButton(QStringLiteral("拉伸到指定面"));
        rbDim->setChecked(true);

        layType->addWidget(rbDim);
        layType->addWidget(rbAll);
        layType->addWidget(rbToFace);
        boxType->setContent(wType);
        mainLayout()->addWidget(boxType);

        // 2. 拉伸参数
        TaskBox* boxParam = new TaskBox(QStringLiteral("拉伸参数"));
        QWidget* wParam = new QWidget;
        QVBoxLayout* layParam = new QVBoxLayout(wParam);

        layParam->addWidget(new QLabel(QStringLiteral("拉伸长度")));
        QDoubleSpinBox* spinLen = new QDoubleSpinBox;
        spinLen->setRange(0.01, 9999.0);
        spinLen->setValue(10.0);
        spinLen->setSingleStep(1.0);
        layParam->addWidget(spinLen);

        layParam->addWidget(new QLabel(QStringLiteral("拉伸方向")));
        QComboBox* cboDir = new QComboBox;
        cboDir->addItems({
            QStringLiteral("正向"),
            QStringLiteral("反向"),
            QStringLiteral("双向对称")
            });
        layParam->addWidget(cboDir);

        boxParam->setContent(wParam);
        mainLayout()->addWidget(boxParam);

        // 3. 布尔运算
        TaskBox* boxBool = new TaskBox(QStringLiteral("布尔运算"));
        QWidget* wBool = new QWidget;
        QVBoxLayout* layBool = new QVBoxLayout(wBool);

        QComboBox* cboBool = new QComboBox;
        cboBool->addItems({
            QStringLiteral("新建实体"),
            QStringLiteral("相加"),
            QStringLiteral("相减"),
            QStringLiteral("相交")
            });
        layBool->addWidget(cboBool);
        layBool->addWidget(new QCheckBox(QStringLiteral("合并相切边")));

        boxBool->setContent(wBool);
        mainLayout()->addWidget(boxBool);

        mainLayout()->addStretch();
    }
    void PadTaskDialog::clickOk()
    {
        SketcherObj* sketchObj = SketcherObjManager::instance().GetCurrentActiveSketcherObj();
        if (sketchObj) {
            Part::TopoShape sketchshape=sketchObj->toShape();
            Part::TopoShape prism;
			double dir[3] = { 1,0,0 };
            sketchObj->getPlaneNormal(dir);
            try {
                Part::ExtrusionParameters params;
                params.taperAngleFwd = 0;
                params.innerWireTaper = Part::InnerWireTaper::SameAsOuter;
                params.dir = gp_Vec(dir[0], dir[1], dir[2]);
                params.solid = true;
                params.lengthFwd = 1.0;
                std::vector<Part::TopoShape> drafts;
                Part::ExtrusionHelper::makeElementDraft(
                    params,
                    sketchshape,
                    drafts, App::StringHasherRef()
                );
                if (drafts.empty()) {
                    return;
                }
                prism.makeElementCompound(
                    drafts,
                    nullptr,
                    Part::TopoShape::SingleShapeCompoundCreationPolicy::returnShape
                );

                //prism.makeElementPrism(sketchshape, 1.0 * gp_Vec(dir[0], dir[1], dir[2]));
                auto& view = GetService(Editor::Panels::SceneView);
                auto scene = view.GetScene();
                auto topoActor = new Core::ECS::TopoActor(scene, "TopoShapePrism", "TopoShape", false);
                const auto& topoComp = topoActor->GetComponent<Core::ECS::Components::CTopoShape>();
                Part::TopoShape& topo = topoComp->GetTopoShape();
                topo.setShape(prism);
                topoComp->discretizationShape();
            }
            catch (Standard_Failure&) {
                throw Base::RuntimeError("FeatureExtrusion: Length: Could not extrude the sketch!");
            }
        }
    }
    void PadTaskDialog::clickApply()
    {
    }
    void PadTaskDialog::clickCancel()
    {
    }
}