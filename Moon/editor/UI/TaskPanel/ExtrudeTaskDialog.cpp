#include "editor/UI/TaskPanel/ExtrudeTaskDialog.h"
#include "TaskBox.h"
#include "Sketcher/SketcherObjManager.h"
#include "Sketcher/SketcherObj.h"
#include "App/ExtrusionHelper.h"
#include "core/component/TopoShapeActor.h"
#include "core/component/CTopoShape.h"
#include "feature/SketcherFeature.h"
#include "feature/ExtrudeFeature.h"
#include "feature/FeatureBody.h"
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
                faceShape = feature->getProfileFace();
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
                //基于已有的feature 
                if (baseFeature) {
                    extrudeFeature->setBaseFeature(baseFeature);
                    extrudeFeature->setSubValues(subValues);
                    faceShape = extrudeFeature->getProfileFace();
                }
                else
                {
                    // 1. 获取当前激活的草图,
                    auto* sketchFeature = SketcherObjManager::instance().GetCurrentActiveSketcherFeature();
                    if (sketchFeature) {
                        //2.设置基于最后一个feature
                        extrudeFeature->setBaseFeature(FeatureBody::instance().getLastBaseFeature());
                        FeatureBody::instance().addFeature(extrudeFeature);
                        extrudeFeature->setProfile(sketchFeature);
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
        PadTaskWidget* behaviour = nullptr;
        ExtrudeType extrudeType;
        ExtrudeTaskDialog* self;
        friend ExtrudeTaskDialog;
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
        //mInternal->behaviour->setAngle(mInternal->spinAngleForward->value());
    }
    void ExtrudeTaskDialog::onLengthChange()
    {
        //mInternal->behaviour->setLength(mInternal->spinLenForward->value());
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