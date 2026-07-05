#include "editor/UI/TaskPanel/ExtrudeTaskDialog.h"
#include "TaskBox.h"
#include "Sketcher/SketcherObjManager.h"
#include "Sketcher/SketcherObj.h"
#include "App/ExtrusionHelper.h"
#include "core/component/TopoShapeActor.h"
#include "core/component/CTopoShape.h"
#include "TopoShape.h"
#include "Core/Global/ServiceLocator.h"
#include "renderer/SceneView.h"
#include "core/ViewTool.h"
#include "core/log.h"
#include "Interactive/Widgets/PadTaskWidget.h"
#include <Core/ECS/Components/CMaterialRenderer.h>
#include <QLabel>
#include <QLineEdit>
#include <QCheckBox>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QGroupBox>
#include <QRadioButton>
#include <numbers>
#include <gp_Pln.hxx>
#include <TopoDS.hxx>
#include <BRepAdaptor_Surface.hxx>
#include <BRepLProp_SLProps.hxx>
#include <BRepGProp_Face.hxx>
namespace MOON {
    class ExtrudeTaskDialog::Internal {
    public:
        Internal(ExtrudeTaskDialog* pad , ExtrudeType type) :self(pad), extrudeType(type){
            behaviour = new PadTaskWidget("pad");
			std::vector<Part::TopoShape> shapes;
            ViewTool::getSelectedTopoShape(shapes);
            if (shapes.size() > 0) {
                faceShape = shapes[1];
				baseShape = shapes[0];
            }
            else
            {
                // 1. 获取当前激活的草图
                SketcherObj* sketchObj = SketcherObjManager::instance().GetCurrentActiveSketcherObj();
                if (sketchObj) {
                     faceShape = sketchObj->getDoneFaceShape();
                     baseShape = sketchObj->getBasedTopoShape();
                }
            }  
            if(!faceShape.isNull())
            {
                gp_Pln pln;
                if (!faceShape.findPlane(pln)) {
                    TopoDS_Face face = TopoDS::Face(faceShape.getShape());
                    BRepAdaptor_Surface adapt(face);
                    double u = adapt.FirstUParameter()
                        + (adapt.LastUParameter() - adapt.FirstUParameter()) / 2.;
                    double v = adapt.FirstVParameter()
                        + (adapt.LastVParameter() - adapt.FirstVParameter()) / 2.;
                    BRepLProp_SLProps prop(adapt, u, v, 2, Precision::Confusion());
                    if (prop.IsNormalDefined()) {
                        gp_Pnt pnt;
                        gp_Vec vec;
                        // handles the orientation state of the shape
                        BRepGProp_Face(face).Normal(u, v, pnt, vec);
                        pln = gp_Pln(pnt, gp_Dir(vec));
                    }
                }
                behaviour->setUpOrigin(pln.Location().X(), pln.Location().Y(), pln.Location().Z());
                if (extrudeType == ExtrudeType::Additive)
                {
                    behaviour->setUpDir(pln.Axis().Direction().X(), pln.Axis().Direction().Y(), pln.Axis().Direction().Z());
                    finalDir = { pln.Axis().Direction().X(), pln.Axis().Direction().Y(), pln.Axis().Direction().Z() };
                }
                else
                {
                    behaviour->setUpDir(-pln.Axis().Direction().X(), -pln.Axis().Direction().Y(), -pln.Axis().Direction().Z());
                    finalDir = { -pln.Axis().Direction().X(), -pln.Axis().Direction().Y(), -pln.Axis().Direction().Z() };
                }
                behaviour->setUpXAxis(pln.XAxis().Direction().X(), pln.XAxis().Direction().Y(), pln.XAxis().Direction().Z());
                behaviour->setUpYAxis(pln.YAxis().Direction().X(), pln.YAxis().Direction().Y(), pln.YAxis().Direction().Z());
                behaviour->setLength(10);
                behaviour->AddObserver(PadTaskEvent::LengthChange, self, &ExtrudeTaskDialog::onWidgetLengthInvoke);
                behaviour->AddObserver(PadTaskEvent::AngleChange, self, &ExtrudeTaskDialog::onWidgetAngleInvoke);
            }
        }
        ~Internal() {
            delete behaviour;
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
    private:
        //based which shape to extrude and use which shape to extrude
        Part::TopoShape baseShape;
        Part::TopoShape faceShape;
        gp_Vec finalDir;

        PadTaskWidget* behaviour = nullptr;
       
        ExtrudeType extrudeType;
        ExtrudeTaskDialog* self;
        friend ExtrudeTaskDialog;
        // 在 ExtrudeTaskDialog.h 中添加
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
    ExtrudeTaskDialog::ExtrudeTaskDialog(QWidget* parent, ExtrudeType type)
        : BaseTaskDialog(parent), mInternal(new Internal(this,type))
    {

        if (type == ExtrudeType::Subtractive) {
            mPreviewOption.isTransparent = false;
            mPreviewOption.isBlend = true;
        }

        setGenerateShapeName("PadShape");
        buildUi();
        previewShape();
    }

    ExtrudeTaskDialog::~ExtrudeTaskDialog()
    {
        delete mInternal;
    }

    bool ExtrudeTaskDialog::generateShape()
    {
		// 1. 检查是否有有效的拉伸面
        if (mInternal->faceShape.isNull()) {
            return false;
        }

        // 2. 获取界面控件（你需要把控件提升为成员变量，我下面会说明）
        //    先假设你已经把 spinLen, cboDir, cboBool, rbDim 等改成成员变量
        if (!mInternal->spinLenForward || !mInternal->cboDir || !mInternal->cboBool || !mInternal->rbDim || !mInternal->rbAll || !mInternal->rbToFace)
            return false;

        // 3. 获取界面参数
        double lengthForward = mInternal->spinLenForward->value();
        double angleForward = mInternal->spinAngleForward->value();
        double lengthRev = mInternal->spinLenRev->value();
        double angleRev = mInternal->spinAngleRev->value();
        int dirType = mInternal->cboDir->currentIndex();       // 0=正向,1=反向,2=对称
        int boolType = mInternal->cboBool->currentIndex();     // 0=新建,1=相加,2=相减,3=相交
        bool isDim = mInternal->rbDim->isChecked();
        bool isAll = mInternal->rbAll->isChecked();
        bool isToFace = mInternal->rbToFace->isChecked();

        // 4. 获取草图平面法向（拉伸方向基准）
        // 5. 根据方向类型调整最终拉伸向量
        Part::ExtrusionParameters params;
        params.taperAngleFwd = angleForward * std::numbers::pi / 180.0;
        params.innerWireTaper = Part::InnerWireTaper::SameAsOuter;
        params.dir = mInternal->finalDir;
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

        // 6. 生成拉伸体(核心)
        Part::TopoShape prism;
        try {
            std::vector<Part::TopoShape> drafts;
            Part::ExtrusionHelper::makeElementDraft(
                params,
                mInternal->faceShape,
                drafts, App::StringHasherRef()
            );
            if (drafts.empty()) {
                return false;
            }
            prism.makeElementCompound(
                drafts,
                nullptr,
                Part::TopoShape::SingleShapeCompoundCreationPolicy::returnShape
            );
            getPreviewShape() = prism;
          
            Part::TopoShape resShape;
            if (!mInternal->baseShape.isNull()) {
                if (mInternal->extrudeType == ExtrudeType::Additive) {
                    resShape = prism.makeElementFuse(mInternal->baseShape);
                }
                else if (mInternal->extrudeType == ExtrudeType::Subtractive) {
                    resShape = mInternal->baseShape.makeElementCut(prism);
                }
            }
            else {
                resShape = prism;
            }
            getGenerateShape() = resShape;
           
           
            return true;
        }
        catch (Base::ValueError e) {
            CORE_ERROR(e.getMessage());
            // 拉伸失败，清空预览
           
            return false;;
        }
        return false;
    }
    void ExtrudeTaskDialog::buildUi()
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
        connect(mInternal->rbDim, &QRadioButton::toggled, this, &ExtrudeTaskDialog::onValueChange);
        connect(mInternal->rbAll, &QRadioButton::toggled, this, &ExtrudeTaskDialog::onValueChange);
        connect(mInternal->rbToFace, &QRadioButton::toggled, this, &ExtrudeTaskDialog::onValueChange);

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
        connect(mInternal->cboDir, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ExtrudeTaskDialog::onValueChange);
        mInternal->labelLen1 = new QLabel(QStringLiteral("拉伸长度1"));
        layParam->addWidget(mInternal->labelLen1);
        mInternal->spinLenForward = new QDoubleSpinBox;
        mInternal->spinLenForward->setRange(1, 9999.0);
        mInternal->spinLenForward->setValue(10);
        mInternal->spinLenForward->setSingleStep(1);
        layParam->addWidget(mInternal->spinLenForward);

        mInternal->labelAngle1 = new QLabel(QStringLiteral("拉伸锥度1"));
        layParam->addWidget(mInternal->labelAngle1);
        mInternal->spinAngleForward = new QDoubleSpinBox;
        mInternal->spinAngleForward->setRange(-90, 90);
        mInternal->spinAngleForward->setValue(0);
        mInternal->spinAngleForward->setSingleStep(1);
        layParam->addWidget(mInternal->spinAngleForward);

        mInternal->labelLen2 = new QLabel(QStringLiteral("拉伸长度2"));
        layParam->addWidget(mInternal->labelLen2);
        mInternal->spinLenRev = new QDoubleSpinBox;
        mInternal->spinLenRev->setRange(0.01, 9999.0);
        mInternal->spinLenRev->setValue(1);
        mInternal->spinLenRev->setSingleStep(1);
        layParam->addWidget(mInternal->spinLenRev);

        mInternal->labelAngle2 = new QLabel(QStringLiteral("拉伸锥度2"));
        layParam->addWidget(mInternal->labelAngle2);
        mInternal->spinAngleRev = new QDoubleSpinBox;
        mInternal->spinAngleRev->setRange(-90, 90);
        mInternal->spinAngleRev->setValue(0);
        mInternal->spinAngleRev->setSingleStep(1);
        layParam->addWidget(mInternal->spinAngleRev);
        boxParam->setContent(wParam);
        mainLayout()->addWidget(boxParam);
        // ==========================
        // 参数变化 → 预览
        // ==========================
        connect(mInternal->spinLenForward, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &ExtrudeTaskDialog::onValueChange);
        connect(mInternal->spinAngleForward, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &ExtrudeTaskDialog::onValueChange);
        connect(mInternal->spinLenForward, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &ExtrudeTaskDialog::onLengthChange);
        connect(mInternal->spinAngleForward, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &ExtrudeTaskDialog::onAngleChange);
        connect(mInternal->spinLenRev, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &ExtrudeTaskDialog::onValueChange);
        connect(mInternal->spinAngleRev, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &ExtrudeTaskDialog::onValueChange);
        connect(mInternal->cboDir, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ExtrudeTaskDialog::onValueChange);
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
        connect(mInternal->cboBool, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ExtrudeTaskDialog::onValueChange);
        connect(mInternal->cbMergeEdges, &QCheckBox::toggled, this, &ExtrudeTaskDialog::onValueChange);

        mainLayout()->addStretch();
    }
    void ExtrudeTaskDialog::clickOk()
    {
        generateFinalShape();
    }
    void ExtrudeTaskDialog::clickApply()
    {
    }
    void ExtrudeTaskDialog::clickCancel()
    {
        clearPreviewShape();
    }
    void ExtrudeTaskDialog::onValueChange()
    {
        previewShape();
        mInternal->updateDirectionUI(mInternal->cboDir->currentIndex());
    }
    void ExtrudeTaskDialog::onAngleChange()
    {
        mInternal->behaviour->setAngle(mInternal->spinAngleForward->value());
    }
    void ExtrudeTaskDialog::onLengthChange()
    {
        mInternal->behaviour->setLength(mInternal->spinLenForward->value());
    }
    void ExtrudeTaskDialog::onWidgetLengthInvoke()
    {
        mInternal->spinLenForward->setValue(mInternal->behaviour->getLength());
    }
    void ExtrudeTaskDialog::onWidgetAngleInvoke()
    {
        mInternal->spinAngleForward->setValue(mInternal->behaviour->getAngle());
    }
}