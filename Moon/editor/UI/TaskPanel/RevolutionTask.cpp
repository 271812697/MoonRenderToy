#include <tracy/Tracy.hpp>
#include "editor/UI/TaskPanel/RevolutionTask.h"
#include "Widgets/SliderFloatProperty.h"
#include "Widgets/EnumProperty.h"
#include "Widgets/BoolProperty.h"
#include "TopoShape.h"
#include "Sketcher/SketcherObjManager.h"
#include "Sketcher/SketcherObj.h"
#include "core/ViewTool.h"
#include "Interactive/Widgets/ArrowRotateWidget.h"

#include <gp_Vec.hxx>
#include <gp_Dir.hxx>

#include <BRepAdaptor_Curve.hxx>
#include <TopoDS.hxx>
#include <GProp_GProps.hxx>
#include <BRepGProp.hxx>
namespace MOON {
    class RevolutionTask::Internal {
    public:
        Internal(RevolutionTask*s):self(s){
            std::vector<Part::TopoShape>shapes;
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
            mBehaviour = new ArrowRotateWidget("rotate");
            mBehaviour->setImmediateInvoke(false);
            if (!faceShape.isNull()) {
                Part::TopoShape tempShape = faceShape.makeElementFace(nullptr, "Part::FaceMakerCheese");
                GProp_GProps props;
                BRepGProp::SurfaceProperties(tempShape.getSubTopoShape(TopAbs_FACE, 1).getShape(), props);
              
                gp_Pnt cog = props.CentreOfMass();
                mBehaviour->setUpOriginPos(cog.X(),cog.Y(),cog.Z()); 
                mBehaviour->AddObserver(ArrowRotateEvent::AngleChange, self, &RevolutionTask::onAngleChange);
            }
        }
        void setAxis(const gp_Ax1& ax) {
            axis = ax;
            gp_Ax1 tempAxis = axis;
            if (reverse) {
                tempAxis.Reverse();
            }
            mBehaviour->setUpRotateAxis(
                tempAxis.Direction().X(), tempAxis.Direction().Y(), tempAxis.Direction().Z());
            mBehaviour->setUpRotateCenter(ax.Location().X(),
                tempAxis.Location().Y(), tempAxis.Location().Z());
            mBehaviour->setAngle(angle);

        }
        bool setSketcherAxis() {
            // 1. 获取当前激活的草图
            SketcherObj* sketchObj = SketcherObjManager::instance().GetCurrentActiveSketcherObj();
            if (sketchObj) {
                Base::Vector3d origin = sketchObj->getPlaneOrigin();
                Base::Vector3d saxis = sketchObj->getPlaneXAxis();
                if (axisType == 1) {
                    saxis = sketchObj->getPlaneYAxis();
                }
                gp_Pnt b(origin.x, origin.y, origin.z);;
                gp_Dir d(saxis.x, saxis.y, saxis.z); 
                setAxis(gp_Ax1(b, d));
                return true;
            }        
            return false;
        }
        void onSelectAny() {
        }
        ~Internal() {
            delete mBehaviour;
        }
    private:
        ArrowRotateWidget* mBehaviour = nullptr;
        SliderFloatProperty* angleProp;
        friend RevolutionTask;
        RevolutionTask* self = nullptr;
        RevolutionType mType;
        Part::TopoShape baseShape;
        Part::TopoShape faceShape;

       
        gp_Ax1 axis;
        int axisType = 0;
        float angle=90;
        bool  reverse = false;
    };

    RevolutionTask::RevolutionTask(RevolutionType type,QWidget* parent, Feature* feature )
        : ParamTaskDialog(parent),ShapeHelper(feature), mInternal(new Internal(this))
    {       
        mInternal->mType = type;
        setGenerateShapeName("RevolutionShape");

        mPreviewOption.isTransparent = true;
        mPreviewOption.isBlend = true;
        mPreviewOption.useDomainColor = false;
        mPreviewOption.g = 0;
        mPreviewOption.b = 0;
        PropertyComponent* p=addGroupParam("Revolve");
        mInternal->angleProp = new SliderFloatProperty("Angle", p);
        mInternal->angleProp->setMinMax(-360,360);

        addParam(mInternal->angleProp);
        EnumProperty* join = new EnumProperty("Axis", p);
        addParam(join);
        BoolProperty* reverse = new BoolProperty("Reverse", p);
        addParam(reverse);
        buildUi();
        initilized = true;
        previewShape();
    }

    RevolutionTask::~RevolutionTask()
    {
        delete mInternal;
    }

    QVariant RevolutionTask::getParamValue(const QString& propertyName)
    {
        if (propertyName == "Revolve:Angle") {
            return QVariant::fromValue(mInternal->angle);
        }
        else if (propertyName == "Revolve:Axis") {
            QList<QString>list = {"Sketch X", "Sketch Y" ,"Select Edge"  };
            return QVariant::fromValue(list);
        }
       
        else if (propertyName == "Revolve:Reverse") {
            return QVariant::fromValue(mInternal->reverse);
        }
        return QVariant();
    }

    void RevolutionTask::setParamValue(const QString& propertyName, const QVariant& value)
    {
        ZoneScoped;
        bool updatePreView = false;
        if (propertyName == "Revolve:Angle") {
            mInternal->angle = value.toFloat();
            mInternal->mBehaviour->setAngle(mInternal->angle);
            updatePreView = true;
        }
        else if (propertyName == "Revolve:Axis") {
            mInternal->axisType = value.value<int>();
            if (mInternal->axisType != 2) {
                if (mInternal->setSketcherAxis()) {
                    updatePreView = true;
                }
            }
        }
        else if (propertyName == "Revolve:Reverse") {
            mInternal->reverse = value.toBool();
			mInternal->setAxis(mInternal->axis);
            updatePreView = true;
        }
        if (updatePreView&&initilized) {
            previewShape();
        }
    }


    void RevolutionTask::clickOk()
    {
        generateFinalShape();
    }
    void RevolutionTask::clickApply()
    {
    }
    void RevolutionTask::clickCancel()
    {
        clearPreviewShape();
    }
    bool RevolutionTask::generateShape()
    {
        ZoneScoped;
        if (mInternal->faceShape.isNull()) {
            return false;
        }
        gp_Ax1 raxis=mInternal->axis;
        if (mInternal->reverse) {
            raxis.Reverse();
        }
        float radAngle = mInternal->angle * 3.14159265358979323846f / 180.0f;
        Part::TopoShape revolve;
        {
			ZoneScopedN("Revolve");
            revolve= mInternal->faceShape.makeElementRevolve(raxis, radAngle, "Part::FaceMakerCheese");
            getPreviewShape() = revolve;
        }
      
        Part::TopoShape resShape;
        if (!mInternal->baseShape.isNull()) {
            ZoneScopedN("makeBoolen");
            if (mInternal->mType == RevolutionType::ReAdditive) {
                resShape = mInternal->baseShape.makeElementFuse(revolve);
            }
            else if (mInternal->mType == RevolutionType::ReSubtractive) {
                resShape = mInternal->baseShape.makeElementCut(revolve);
            }
        }
        else
        {
            resShape = revolve;
        }
        getGenerateShape() = resShape;
        return true;
    }
    void RevolutionTask::onSelectEdge(const std::vector<Part::TopoShape>& edge)
    {  
        ZoneScoped;
        if (mInternal->axisType == 2) {
            TopoDS_Edge refEdge = TopoDS::Edge(edge[1].getShape());
            BRepAdaptor_Curve adapt(refEdge);
            gp_Pnt b;
            gp_Dir d;
            GeomAbs_CurveType curveType = adapt.GetType();
            if (curveType == GeomAbs_Line) {
                b = adapt.Line().Location();
                d = adapt.Line().Direction();
            }
            else if (curveType == GeomAbs_Circle) {
                b = adapt.Circle().Location();
                d = adapt.Circle().Axis().Direction();
            }
            else
            {
                double middle = adapt.FirstParameter() + adapt.LastParameter();
                gp_Vec v;
                adapt.D1(middle, b, v);
                d = v;
            }
            mInternal->setAxis(gp_Ax1(b, d));
            previewShape();
        }
    }
    void RevolutionTask::onAngleChange()
    {
        ZoneScoped;
        mInternal->angle= mInternal->mBehaviour->getAngle();
        mInternal->angleProp->updateWidgetValue(mInternal->angle);
    }
}