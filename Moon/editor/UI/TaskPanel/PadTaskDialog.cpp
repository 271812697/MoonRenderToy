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
#include "core/log.h"
#include <Core/ECS/Components/CMaterialRenderer.h>
#include <QLabel>
#include <QLineEdit>
#include <QCheckBox>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QGroupBox>
#include <QRadioButton>
#include <numbers>
namespace MOON {
    class PadTaskDialog::Internal {
    public:
        Internal(PadTaskDialog* pad) :self(pad) {
        }
        ~Internal() {
            if (m_previewActor != nullptr) {
                m_previewActor->RemoveFromScene();
                delete m_previewActor;
                m_previewActor = nullptr;
            }
        }
        void updateDirectionUI(int dirType) {
            if (dirType == 0) { // 正向
                labelLen2->hide();
                spinLenRev->hide();
                labelAngle2->hide();
                spinAngleRev->hide();
            }
            else if (dirType == 1) { // 双向
                labelLen2->show();
                spinLenRev->show();
                labelAngle2->show();
                spinAngleRev->show();
            }
            else if (dirType == 2) { // 对称
                labelLen2->hide();
                spinLenRev->hide();
                labelAngle2->hide();
                spinAngleRev->hide();
            }
        }
        bool prismSketch() {

            // 1. 获取当前激活的草图
            SketcherObj* sketchObj = SketcherObjManager::instance().GetCurrentActiveSketcherObj();
            if (!sketchObj)
                return false;

            // 2. 获取界面控件（你需要把控件提升为成员变量，我下面会说明）
            //    先假设你已经把 spinLen, cboDir, cboBool, rbDim 等改成成员变量
            if (!spinLenForward || !cboDir || !cboBool || !rbDim || !rbAll || !rbToFace)
                return false;

            // 3. 获取界面参数
            double lengthForward = spinLenForward->value();
            double angleForward = spinAngleForward->value();
            double lengthRev = spinLenRev->value();
            double angleRev = spinAngleRev->value();
            int dirType = cboDir->currentIndex();       // 0=正向,1=反向,2=对称
            int boolType = cboBool->currentIndex();     // 0=新建,1=相加,2=相减,3=相交
            bool isDim = rbDim->isChecked();
            bool isAll = rbAll->isChecked();
            bool isToFace = rbToFace->isChecked();

            // 4. 获取草图平面法向（拉伸方向基准）
            double dir[3] = { 0,0,1 };
            sketchObj->getPlaneNormal(dir);
            gp_Vec baseDir(dir[0], dir[1], dir[2]);
            gp_Vec finalDir = baseDir;
            // 5. 根据方向类型调整最终拉伸向量
            Part::ExtrusionParameters params;
            params.taperAngleFwd = angleForward * std::numbers::pi / 180.0;
            params.innerWireTaper = Part::InnerWireTaper::SameAsOuter;
            params.dir = finalDir;
            params.solid = true;
            params.lengthFwd = lengthForward;

            if (dirType == 0)      // 正向
            {

            }
            else if (dirType == 1) // 双向
            {
                params.lengthRev = lengthRev;
                params.taperAngleRev = angleRev * std::numbers::pi / 180.0;

            }
            else if (dirType == 2) // 对称
            {
                params.lengthRev = params.lengthFwd;
                params.taperAngleRev = params.taperAngleFwd * std::numbers::pi / 180.0;
            }

            // 6. 生成拉伸体（核心）
            Part::TopoShape sketchShape = sketchObj->toShape();
            try {
                std::vector<Part::TopoShape> drafts;
                Part::ExtrusionHelper::makeElementDraft(
                    params,
                    sketchShape,
                    drafts, App::StringHasherRef()
                );
                if (drafts.empty()) {
                    return false;
                }
                m_previewShape.makeElementCompound(
                    drafts,
                    nullptr,
                    Part::TopoShape::SingleShapeCompoundCreationPolicy::returnShape
                );
                return true;
            }
            catch (...) {
                // 拉伸失败，清空预览
                clearPreview();
                return false;;
            }
            return false;
        }
        void previewShape()
        {
            if (prismSketch()) {
                if (m_previewActor == nullptr) {
                    auto& view = GetService(Editor::Panels::SceneView);
                    auto scene = view.GetScene();
                    m_previewActor = new Core::ECS::TopoActor(scene, "TopoShapePrismPreview", "TopoShape", true);
                }
                m_previewActor->ClearModel();

                const auto& topoComp = m_previewActor->GetComponent<Core::ECS::Components::CTopoShape>();
                Part::TopoShape& topo = topoComp->GetTopoShape();
                topo.setShape(m_previewShape);
                topoComp->discretizationShape();
                auto MatRender = m_previewActor->GetComponent<Core::ECS::Components::CMaterialRenderer>();
                Core::Resources::Material* tempMat = MatRender->GetMaterialAtIndex(0);
                tempMat->SetProperty("u_Albedo", Maths::FVector4(1, 1, 1, 0.4));
                tempMat->SetBlendable(true);
                tempMat->SetDepthWriting(true);
            }
        }

        // 清理旧预览
        void clearPreview()
        {
        }

    private:
        PadTaskDialog* self;
        friend PadTaskDialog;
        // 在 PadTaskDialog.h 中添加
        QDoubleSpinBox* spinLenForward = nullptr;
        QDoubleSpinBox* spinAngleForward = nullptr;
        QDoubleSpinBox* spinLenRev = nullptr;
        QDoubleSpinBox* spinAngleRev = nullptr;
        QLabel* labelLen2 = nullptr;
        QLabel* labelAngle2 = nullptr;
        QLabel* labelLen1 = nullptr;
        QLabel* labelAngle1 = nullptr;
        QComboBox* cboDir = nullptr;
        QComboBox* cboBool = nullptr;
        QRadioButton* rbDim = nullptr;
        QRadioButton* rbAll = nullptr;
        QRadioButton* rbToFace = nullptr;
        QCheckBox* cbMergeEdges = nullptr;
        // 预览用的临时形状
        Part::TopoShape m_previewShape;
        // 预览用的Actor
        Core::ECS::TopoActor* m_previewActor = nullptr;
    };
    PadTaskDialog::PadTaskDialog(QWidget* parent)
        : BaseTaskDialog(parent), mInternal(new Internal(this))
    {
        buildUi();
        previewShape();
    }

    PadTaskDialog::~PadTaskDialog()
    {
        delete mInternal;
    }

    void PadTaskDialog::previewShape()
    {
        CORE_INFO("previewShape");
        mInternal->previewShape();
        mInternal->updateDirectionUI(mInternal->cboDir->currentIndex());
    }

    void PadTaskDialog::buildUi()
    {
        // 1. 拉伸类型
        TaskBox* boxType = new TaskBox(QStringLiteral("拉伸类型"));
        QWidget* wType = new QWidget;
        QVBoxLayout* layType = new QVBoxLayout(wType);
        layType->setContentsMargins(0, 0, 0, 0);
        layType->setSpacing(6);

        mInternal->rbDim = new QRadioButton(QStringLiteral("给定长度"));
        mInternal->rbAll = new QRadioButton(QStringLiteral("贯穿全部"));
        mInternal->rbToFace = new QRadioButton(QStringLiteral("拉伸到指定面"));
        mInternal->rbDim->setChecked(true);

        layType->addWidget(mInternal->rbDim);
        layType->addWidget(mInternal->rbAll);
        layType->addWidget(mInternal->rbToFace);
        boxType->setContent(wType);
        mainLayout()->addWidget(boxType);

        // ==========================
        // 拉伸类型变化 → 预览
        // ==========================
        connect(mInternal->rbDim, &QRadioButton::toggled, this, &PadTaskDialog::previewShape);
        connect(mInternal->rbAll, &QRadioButton::toggled, this, &PadTaskDialog::previewShape);
        connect(mInternal->rbToFace, &QRadioButton::toggled, this, &PadTaskDialog::previewShape);

        // 2. 拉伸参数
        TaskBox* boxParam = new TaskBox(QStringLiteral("拉伸参数"));
        QWidget* wParam = new QWidget;
        QVBoxLayout* layParam = new QVBoxLayout(wParam);

        layParam->addWidget(new QLabel(QStringLiteral("拉伸方向")));
        mInternal->cboDir = new QComboBox;
        mInternal->cboDir->addItems({
            QStringLiteral("正向"),
            QStringLiteral("双向"),
            QStringLiteral("对称")
            });
        layParam->addWidget(mInternal->cboDir);
        connect(mInternal->cboDir, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int idx) {
            mInternal->updateDirectionUI(idx);
            });
        mInternal->labelLen1 = new QLabel(QStringLiteral("拉伸长度1"));
        layParam->addWidget(mInternal->labelLen1);
        mInternal->spinLenForward = new QDoubleSpinBox;
        mInternal->spinLenForward->setRange(0.01, 9999.0);
        mInternal->spinLenForward->setValue(0.5);
        mInternal->spinLenForward->setSingleStep(0.05);
        layParam->addWidget(mInternal->spinLenForward);

        mInternal->labelAngle1 = new QLabel(QStringLiteral("拉伸锥度1"));
        layParam->addWidget(mInternal->labelAngle1);
        mInternal->spinAngleForward = new QDoubleSpinBox;
        mInternal->spinAngleForward->setRange(-90, 90);
        mInternal->spinAngleForward->setValue(0);
        mInternal->spinAngleForward->setSingleStep(0.5);
        layParam->addWidget(mInternal->spinAngleForward);

        mInternal->labelLen2 = new QLabel(QStringLiteral("拉伸长度2"));
        layParam->addWidget(mInternal->labelLen2);
        mInternal->spinLenRev = new QDoubleSpinBox;
        mInternal->spinLenRev->setRange(0.01, 9999.0);
        mInternal->spinLenRev->setValue(0.5);
        mInternal->spinLenRev->setSingleStep(0.05);
        layParam->addWidget(mInternal->spinLenRev);

        mInternal->labelAngle2 = new QLabel(QStringLiteral("拉伸锥度2"));
        layParam->addWidget(mInternal->labelAngle2);
        mInternal->spinAngleRev = new QDoubleSpinBox;
        mInternal->spinAngleRev->setRange(-90, 90);
        mInternal->spinAngleRev->setValue(0);
        mInternal->spinAngleRev->setSingleStep(0.5);
        layParam->addWidget(mInternal->spinAngleRev);



        boxParam->setContent(wParam);
        mainLayout()->addWidget(boxParam);

        // ==========================
        // 参数变化 → 预览
        // ==========================
        connect(mInternal->spinLenForward, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &PadTaskDialog::previewShape);
        connect(mInternal->spinAngleForward, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &PadTaskDialog::previewShape);
        connect(mInternal->spinLenRev, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &PadTaskDialog::previewShape);
        connect(mInternal->spinAngleRev, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &PadTaskDialog::previewShape);
        connect(mInternal->cboDir, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &PadTaskDialog::previewShape);

        // 3. 布尔运算
        TaskBox* boxBool = new TaskBox(QStringLiteral("布尔运算"));
        QWidget* wBool = new QWidget;
        QVBoxLayout* layBool = new QVBoxLayout(wBool);

        mInternal->cboBool = new QComboBox;
        mInternal->cboBool->addItems({
            QStringLiteral("新建实体"),
            QStringLiteral("相加"),
            QStringLiteral("相减"),
            QStringLiteral("相交")
            });
        layBool->addWidget(mInternal->cboBool);

        mInternal->cbMergeEdges = new QCheckBox(QStringLiteral("合并相切边"));
        layBool->addWidget(mInternal->cbMergeEdges);

        boxBool->setContent(wBool);
        mainLayout()->addWidget(boxBool);

        // ==========================
        // 布尔/勾选框变化 → 预览
        // ==========================
        connect(mInternal->cboBool, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &PadTaskDialog::previewShape);
        connect(mInternal->cbMergeEdges, &QCheckBox::toggled, this, &PadTaskDialog::previewShape);

        mainLayout()->addStretch();
    }
    void PadTaskDialog::clickOk()
    {
        if (mInternal->m_previewShape.isNull()) {
            mInternal->prismSketch();
        }
        if (!mInternal->m_previewShape.isNull()) {
            auto& view = GetService(Editor::Panels::SceneView);
            auto scene = view.GetScene();
            auto topoActor = new Core::ECS::TopoActor(scene, "TopoShapePrism", "TopoShape", false);
            const auto& topoComp = topoActor->GetComponent<Core::ECS::Components::CTopoShape>();
            Part::TopoShape& topo = topoComp->GetTopoShape();
            topo.setShape(mInternal->m_previewShape);
            topoComp->discretizationShape();
        }
    }
    void PadTaskDialog::clickApply()
    {
    }
    void PadTaskDialog::clickCancel()
    {
    }
}