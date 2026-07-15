#include "editor/UI/TaskPanel/ExtrudeTaskDialog.h"
#include "TaskBox.h"
#include "Sketcher/SketcherObjManager.h"
#include "Sketcher/SketcherObj.h"
#include "App/ExtrusionHelper.h"
#include "core/component/TopoShapeActor.h"
#include "core/component/CTopoShape.h"
#include "feature/SketcherFeature.h"
#include "feature/ExtrudeFeature.h"
#include "TopoShape.h"
#include "Core/Global/ServiceLocator.h"
#include "renderer/SceneView.h"
#include "core/ViewTool.h"
#include "core/log.h"
#include "Interactive/Widgets/PadTaskWidget.h"
#include <Core/ECS/Components/CMaterialRenderer.h>
#include "Widgets/SliderFloatProperty.h"
#include "Widgets/EnumProperty.h"
#include "Widgets/BoolProperty.h"
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
            setUp();
        }
        ~Internal() {
            delete behaviour;
        }
        void setUp() {
            behaviour = new PadTaskWidget("pad");
            Part::TopoShape faceShape;
            auto f = self->getFeature();
            if (f) {
                //执行已有的feature 参数
                feature = dynamic_cast<ExtrudeFeature*>(f);
                faceShape = feature->getVerifyTopoFace();
            }
            else
            {
                //新建一个feature 
                ExtrudeFeature* extrudeFeature = new ExtrudeFeature("extrude",extrudeType== ExtrudeType::Additive?0:1);
                feature = extrudeFeature;
                self->setFeature(extrudeFeature);
                Feature* baseFeature = nullptr;
                std::vector<std::string>subValues;
                ViewTool::getSelectedBasedFeature(baseFeature,subValues);

                if (baseFeature) {
                    extrudeFeature->setBaseFeature(baseFeature);
                    extrudeFeature->setSubValues(subValues);
                    faceShape = extrudeFeature->getVerifyTopoFace();
                }
                else
                {
                    // 1. 获取当前激活的草图
                    auto* sketchFeature = SketcherObjManager::instance().GetCurrentActiveSketcherFeature();
                    if (sketchFeature) {
                        extrudeFeature->setBaseFeature(sketchFeature);
                        faceShape = sketchFeature->getSketcherObj()->getDoneFaceShape();
                    }
                }
            }
            if (!faceShape.isNull())
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
                    feature->finalDir = { pln.Axis().Direction().X(), pln.Axis().Direction().Y(), pln.Axis().Direction().Z() };
                }
                else
                {
                    behaviour->setUpDir(-pln.Axis().Direction().X(), -pln.Axis().Direction().Y(), -pln.Axis().Direction().Z());
                    feature->finalDir = { -pln.Axis().Direction().X(), -pln.Axis().Direction().Y(), -pln.Axis().Direction().Z() };
                }
                behaviour->setUpXAxis(pln.XAxis().Direction().X(), pln.XAxis().Direction().Y(), pln.XAxis().Direction().Z());
                behaviour->setUpYAxis(pln.YAxis().Direction().X(), pln.YAxis().Direction().Y(), pln.YAxis().Direction().Z());
                behaviour->setLength(10);
                behaviour->AddObserver(PadTaskEvent::LengthChange, self, &ExtrudeTaskDialog::onWidgetLengthInvoke);
                behaviour->AddObserver(PadTaskEvent::AngleChange, self, &ExtrudeTaskDialog::onWidgetAngleInvoke);
            }
        }
      
    private:
        ExtrudeFeature* feature=nullptr;
        //based which shape to extrude and use which shape to extrude
       

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
    };
    ExtrudeTaskDialog::ExtrudeTaskDialog(QWidget* parent, ExtrudeType type, Feature* feature)
        : ParamTaskDialog(parent), mInternal(new Internal(this,type)),ShapeHelper(feature)
    {
        if (type == ExtrudeType::Subtractive) {
            mPreviewOption.isTransparent = false;
            mPreviewOption.isBlend = true;
        }
        setGenerateShapeName("PadShape");
        PropertyComponent* p = addGroupParam("Extrude");

        EnumProperty* mode = new EnumProperty("Extrude Type", p);
        addParam(mode);
        EnumProperty* dir = new EnumProperty("Extrude Direction", p); 
        addParam(dir);

        auto extrudeLength1 = new SliderFloatProperty("Length 1", p);
        extrudeLength1->setMinMax(0.1, 9999);
        addParam(extrudeLength1);
        auto extrudeAngle1 = new SliderFloatProperty("Angle 1", p);
        extrudeAngle1->setMinMax(-90,90);
        addParam(extrudeAngle1);

        auto extrudeLength2 = new SliderFloatProperty("Length 2", p);
        extrudeLength2->setMinMax(0.1, 9999);
        addParam(extrudeLength2);
        auto extrudeAngle2 = new SliderFloatProperty("Angle 2", p);
        extrudeAngle2->setMinMax(-90, 90);
        addParam(extrudeAngle2);
        buildUi();
        previewShape();
    }
    QVariant ExtrudeTaskDialog::getParamValue(const QString& propertyName)
    {
        if (propertyName == "Extrude:Extrude Type") {
            QList<QString>list = { "Length", "Through All", "UpToFace" };
            return QVariant::fromValue(list);
        }
        else if (propertyName == "Extrude:Extrude Direction") {
            QList<QString>list = { "Forward", "Reverse", "Double","Sysmetric"};
            return QVariant::fromValue(list);
        }
        else if (propertyName == "Extrude:Length 1") {
            return QVariant::fromValue(mInternal->feature->lengthForward);
        }
        else if (propertyName == "Extrude:Angle 1") {
            return QVariant::fromValue(mInternal->feature->angleForward);
        }
        else if (propertyName == "Extrude:Length 2") {
            return QVariant::fromValue(mInternal->feature->lengthRev);
        }
        else if (propertyName == "Extrude:Angle 2") {
            return QVariant::fromValue(mInternal->feature->angleRev);
        }
        return QVariant();
    }

    void ExtrudeTaskDialog::setParamValue(const QString& propertyName, const QVariant& value)
    {
        bool updatePreView = false;

        if (propertyName == "Extrude:Extrude Type") {
            mInternal->feature->extrudeType = value.value<int>();
            updatePreView = true;
        }
        else if (propertyName == "Extrude:Extrude Direction") {
            mInternal->feature->dirType = value.value<int>();
            updatePreView = true;
        }
        else if (propertyName == "Extrude:Length 1") {
            mInternal->feature->lengthForward = value.toFloat();
            updatePreView = true;
        }
        else if (propertyName == "Extrude:Angle 1") {
            mInternal->feature->angleForward= value.toFloat();
            updatePreView = true;
        }
        else if (propertyName == "Extrude:Length 2") {
            mInternal->feature->lengthRev = value.toFloat();
            updatePreView = true;
        }
        else if (propertyName == "Extrude:Angle 2") {
            mInternal->feature->angleRev= value.toFloat();
            updatePreView = true;
        }

        if (updatePreView&& hasInitUi) {
            previewShape();
        }
    }
    ExtrudeTaskDialog::~ExtrudeTaskDialog()
    {
        delete mInternal;
    }

    bool ExtrudeTaskDialog::generateShape()
    {
		//// 1. 检查是否有有效的拉伸面
  //      if (mInternal->faceShape.isNull()) {
  //          return false;
  //      }

  //      // 2. 获取界面控件（你需要把控件提升为成员变量，我下面会说明）
  //      //    先假设你已经把 spinLen, cboDir, cboBool, rbDim 等改成成员变量
  //      if (!mInternal->spinLenForward || !mInternal->cboDir || !mInternal->cboBool || !mInternal->rbDim || !mInternal->rbAll || !mInternal->rbToFace)
  //          return false;

  //      // 3. 获取界面参数
  //      double lengthForward = mInternal->spinLenForward->value();
  //      double angleForward = mInternal->spinAngleForward->value();
  //      double lengthRev = mInternal->spinLenRev->value();
  //      double angleRev = mInternal->spinAngleRev->value();
  //      int dirType = mInternal->cboDir->currentIndex();       // 0=正向,1=反向,2=双向,3=对称
  //      int boolType = mInternal->cboBool->currentIndex();     // 0=新建,1=相加,2=相减,3=相交
  //      bool isDim = mInternal->rbDim->isChecked();
  //      bool isAll = mInternal->rbAll->isChecked();
  //      bool isToFace = mInternal->rbToFace->isChecked();

  //      // 4. 获取草图平面法向（拉伸方向基准）
  //      // 5. 根据方向类型调整最终拉伸向量
  //      Part::ExtrusionParameters params;
  //      params.taperAngleFwd = angleForward * std::numbers::pi / 180.0;
  //      params.innerWireTaper = Part::InnerWireTaper::SameAsOuter;
  //      params.dir = mInternal->finalDir;
  //      params.solid = true;
  //      params.lengthFwd = lengthForward;

  //      if (dirType == 0)      // 正向
  //      {
  //          
  //      }
  //      else if (dirType == 1) {
  //          params.lengthFwd *= -1;
  //      }
  //      else if (dirType == 2) // 双向
  //      {
  //          params.lengthRev = lengthRev;
  //          params.taperAngleRev = angleRev * std::numbers::pi / 180.0;

  //      }
  //      else if (dirType == 3) // 对称
  //      {
  //          params.lengthRev = params.lengthFwd;
  //          params.taperAngleRev = params.taperAngleFwd * std::numbers::pi / 180.0;
  //      }

  //      // 6. 生成拉伸体(核心)
  //      Part::TopoShape prism;
  //      if (isToFace&&!mInternal->upToFace.isNull()) {
  //          try
  //          {
  //              Part::TopoShape tempShape = mInternal->faceShape.makeElementFace(nullptr, "Part::FaceMakerCheese");
  //              prism=prism.makeElementPrismUntil(
  //                  tempShape,
  //                  mInternal->supportShape,
  //                  mInternal->upToFace, -params.dir ,Part::TopoShape::PrismMode::None,
  //                  true);
  //              if (prism.isNull()) {
  //                  CORE_ERROR("Prim is Null");
  //                  return false;
  //              }
  //              getPreviewShape() = prism;

  //              Part::TopoShape resShape;
  //              if (!mInternal->baseShape.isNull()) {
  //                  if (mInternal->extrudeType == ExtrudeType::Additive) {
  //                      resShape = prism.makeElementFuse(mInternal->baseShape);
  //                  }
  //                  else if (mInternal->extrudeType == ExtrudeType::Subtractive) {
  //                      resShape = mInternal->baseShape.makeElementCut(prism);
  //                  }
  //              }
  //              else {
  //                  resShape = prism;
  //              }
  //              getGenerateShape() = resShape;


  //              return true;
  //          }
  //          catch (const std::exception&)
  //          {
  //              return false;
  //          }
  //      }
  //      else
  //      {
  //         try {
  //              std::vector<Part::TopoShape> drafts;
  //              Part::ExtrusionHelper::makeElementDraft(
  //                  params,
  //                  mInternal->faceShape,
  //                  drafts, App::StringHasherRef()
  //              );
  //              if (drafts.empty()) {
  //                  return false;
  //              }
  //              prism.makeElementCompound(
  //                  drafts,
  //                  nullptr,
  //                  Part::TopoShape::SingleShapeCompoundCreationPolicy::returnShape
  //              );
  //          
  //              getPreviewShape() = prism;
  //        
  //              Part::TopoShape resShape;
  //              if (!mInternal->baseShape.isNull()) {
  //                  if (mInternal->extrudeType == ExtrudeType::Additive) {
  //                      resShape = prism.makeElementFuse(mInternal->baseShape);
  //                  }
  //                  else if (mInternal->extrudeType == ExtrudeType::Subtractive) {
  //                      resShape = mInternal->baseShape.makeElementCut(prism);
  //                  }
  //              }
  //              else {
  //                  resShape = prism;
  //              }
  //              getGenerateShape() = resShape;
  //         
  //         
  //              return true;
  //          }
  //          catch (Base::ValueError e) {
  //              CORE_ERROR(e.getMessage());
  //              // 拉伸失败，清空预览
  //         
  //              return false;;
  //          }
  //      }
        return false;
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
    }
    void ExtrudeTaskDialog::onAngleChange()
    {
        mInternal->behaviour->setAngle(mInternal->spinAngleForward->value());
    }
    void ExtrudeTaskDialog::onLengthChange()
    {
        mInternal->behaviour->setLength(mInternal->spinLenForward->value());
    }
 
    void ExtrudeTaskDialog::onSelectFace(const std::vector<Part::TopoShape>& face)
    {
        ExtrudeFeature* extrudeFeature=
        dynamic_cast<ExtrudeFeature*>(getFeature());
        if (extrudeFeature) {
            extrudeFeature->upToFace = face[1];
        }
        
    }
    void ExtrudeTaskDialog::onWidgetLengthInvoke()
    {
       // mInternal->spinLenForward->setValue(mInternal->behaviour->getLength());
    }
    void ExtrudeTaskDialog::onWidgetAngleInvoke()
    {
        //mInternal->spinAngleForward->setValue(mInternal->behaviour->getAngle());
    }
}