#include "editor/UI/TaskPanel/ExtrudeTaskDialog.h"
#include "Sketcher/SketcherObjManager.h"
#include "Sketcher/SketcherObj.h"
#include "core/component/TopoShapeActor.h"
#include "feature/SketcherFeature.h"
#include "feature/ExtrudeFeature.h"
#include "feature/FeatureBody.h"
#include "TopoShape.h"
#include "core/ViewTool.h"
#include "core/log.h"
#include "Interactive/Widgets/PadTaskWidget.h"
#include "Widgets/SliderFloatProperty.h"
#include "Widgets/EnumProperty.h"

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
            delete behaviour1;
            delete behaviour2;
        }
        void setUp() {
            behaviour1 = new PadTaskWidget("pad");
            behaviour2=new PadTaskWidget("pad");
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
                isCreateFeature = true;
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
                    auto* sketchFeature = SketcherObjManager::instance().GetLastSketcherFeature();
                    if (sketchFeature) {
                        //2.设置基于最后一个feature
                        FeatureBody::instance().setBaseFeatureFor(extrudeFeature);
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
                behaviour1->setUpOrigin(pln.Location().X(), pln.Location().Y(), pln.Location().Z());
                if (extrudeType == ExtrudeType::Additive)
                {
                    behaviour1->setUpDir(pln.Axis().Direction().X(), pln.Axis().Direction().Y(), pln.Axis().Direction().Z());
                    behaviour2->setUpDir(-pln.Axis().Direction().X(), -pln.Axis().Direction().Y(), -pln.Axis().Direction().Z());
                    feature->finalDir = { pln.Axis().Direction().X(), pln.Axis().Direction().Y(), pln.Axis().Direction().Z() };
                }
                else
                { 
                    behaviour1->setUpDir(-pln.Axis().Direction().X(), -pln.Axis().Direction().Y(), -pln.Axis().Direction().Z());
                    behaviour2->setUpDir(pln.Axis().Direction().X(), pln.Axis().Direction().Y(), pln.Axis().Direction().Z());
                    feature->finalDir = { -pln.Axis().Direction().X(), -pln.Axis().Direction().Y(), -pln.Axis().Direction().Z() };
                }
                behaviour1->setUpXAxis(pln.XAxis().Direction().X(), pln.XAxis().Direction().Y(), pln.XAxis().Direction().Z());
                behaviour1->setUpYAxis(pln.YAxis().Direction().X(), pln.YAxis().Direction().Y(), pln.YAxis().Direction().Z());
                behaviour1->setLength(10);
                behaviour1->AddObserver(PadTaskEvent::LengthChange, self, &ExtrudeTaskDialog::onWidgetLengthInvoke1);
                behaviour1->AddObserver(PadTaskEvent::AngleChange, self, &ExtrudeTaskDialog::onWidgetAngleInvoke1);

                behaviour2->setUpXAxis(pln.XAxis().Direction().X(), pln.XAxis().Direction().Y(), pln.XAxis().Direction().Z());
                behaviour2->setUpYAxis(pln.YAxis().Direction().X(), pln.YAxis().Direction().Y(), pln.YAxis().Direction().Z());
                behaviour2->setLength(10);
                behaviour2->AddObserver(PadTaskEvent::LengthChange, self, &ExtrudeTaskDialog::onWidgetLengthInvoke2);
                behaviour2->AddObserver(PadTaskEvent::AngleChange, self, &ExtrudeTaskDialog::onWidgetAngleInvoke2);
            }
        }
    private:
        ExtrudeFeature* feature=nullptr;
        PadTaskWidget* behaviour1 = nullptr;
        PadTaskWidget* behaviour2 = nullptr;
        ExtrudeType extrudeType;
        ExtrudeTaskDialog* self;
        SliderFloatProperty* extrudeLength1 = nullptr;
        SliderFloatProperty* extrudeAngle1 = nullptr;
        SliderFloatProperty* extrudeLength2 = nullptr;
        SliderFloatProperty* extrudeAngle2 = nullptr;
        friend ExtrudeTaskDialog;
        bool isCreateFeature = false;
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

        mInternal->extrudeLength1 = new SliderFloatProperty("Length 1", p);
        mInternal->extrudeLength1->setMinMax(0.1, 1000);
        mInternal->extrudeLength1->setStep(0.1);
        addParam(mInternal->extrudeLength1);
        mInternal->extrudeAngle1 = new SliderFloatProperty("Angle 1", p);
        mInternal->extrudeAngle1->setMinMax(-90,90);
        mInternal->extrudeAngle1->setStep(1.0);
        addParam(mInternal->extrudeAngle1);

        mInternal->extrudeLength2 = new SliderFloatProperty("Length 2", p);
        mInternal->extrudeLength2->setMinMax(0.1, 1000);
        mInternal->extrudeLength2->setStep(0.1);
        addParam(mInternal->extrudeLength2);
        mInternal->extrudeAngle2 = new SliderFloatProperty("Angle 2", p);
        mInternal->extrudeAngle2->setMinMax(-90, 90);
        mInternal->extrudeAngle2->setStep(1.0);
        addParam(mInternal->extrudeAngle2);
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
            if (mInternal->behaviour2) {  
                if (mInternal->feature->dirType == 2) {
                    mInternal->behaviour2->setActive(true);
                }
                else
                {
                    mInternal->behaviour2->setActive(false);
                }
            }
            updatePreView = true;
        }
        else if (propertyName == "Extrude:Length 1") {
            mInternal->feature->lengthForward = value.toFloat();
            if (mInternal->behaviour1)
                mInternal->behaviour1->setLength(mInternal->feature->lengthForward);
            updatePreView = true;
        }
        else if (propertyName == "Extrude:Angle 1") {
            mInternal->feature->angleForward= value.toFloat();
            if(mInternal->behaviour1)
            mInternal->behaviour1->setAngle(mInternal->feature->angleForward);
            updatePreView = true;
        }
        else if (propertyName == "Extrude:Length 2") {
            mInternal->feature->lengthRev = value.toFloat();
            if (mInternal->behaviour2)
                mInternal->behaviour2->setLength(mInternal->feature->lengthRev);
            if (mInternal->feature->dirType == 2) {
                updatePreView = true;
            }
        }
        else if (propertyName == "Extrude:Angle 2") {
            mInternal->feature->angleRev= value.toFloat();
            if (mInternal->behaviour2)
                mInternal->behaviour2->setAngle(mInternal->feature->angleRev);
            if (mInternal->feature->dirType == 2) {
                updatePreView = true;
            }
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
        if (mInternal->isCreateFeature) {
            mInternal->feature->RemoveFromScene();
            delete mInternal->feature;
        }
    }
    void ExtrudeTaskDialog::onSelectFace(const std::vector<Part::TopoShape>& face)
    {
        ExtrudeFeature* extrudeFeature=
        dynamic_cast<ExtrudeFeature*>(getFeature());
        if (extrudeFeature) {
            extrudeFeature->upToFace = face[1];
            if (mInternal->feature->extrudeType == 2) {
                previewShape();
            }
        }
    }
    void ExtrudeTaskDialog::onWidgetLengthInvoke1()
    {
        mInternal->extrudeLength1->updateWidgetValue(mInternal->behaviour1->getLength());
    }
    void ExtrudeTaskDialog::onWidgetAngleInvoke1()
    {
        mInternal->extrudeAngle1->updateWidgetValue(mInternal->behaviour1->getAngle());
    }
    void ExtrudeTaskDialog::onWidgetLengthInvoke2()
    {
        mInternal->extrudeLength2->updateWidgetValue(mInternal->behaviour2->getLength());
    }

    void ExtrudeTaskDialog::onWidgetAngleInvoke2()
    {
        mInternal->extrudeAngle2->updateWidgetValue(mInternal->behaviour2->getAngle());
    }
}