#include "editor/UI/TaskPanel/RevolutionTask.h"
#include "TaskBox.h"
#include "Widgets/SliderFloatProperty.h"
#include "Widgets/EnumProperty.h"
#include "Widgets/BoolProperty.h"
#include "TopoShape.h"
#include "Sketcher/SketcherObjManager.h"
#include "Sketcher/SketcherObj.h"
#include "core/ViewTool.h"
#include "core/log.h"
#include "core/SelectionManager.h"
#include "Interactive/Widgets/AxisTranslationWidget.h"
#include "Interactive/Interactive/ExecuteCommand.h"
#include "Interactive/Widgets/ArrowRotateWidget.h"
#include "base/BoundBox.h"
#include "Geometry.h"

#include <BRepTools.hxx>
#include <BRepAdaptor_Surface.hxx>
#include <gp_Vec.hxx>
#include <gp_Dir.hxx>
#include <GeomAbs_Shape.hxx>
#include <ShapeFix_ShapeTolerance.hxx>
#include <BRepAlgo.hxx>
#include <ShapeAnalysis_Surface.hxx>
#include <BRepAdaptor_Surface.hxx>
#include <BRepLProp_SLProps.hxx>
#include <Precision.hxx>
#include <BRepAdaptor_Surface.hxx>
#include <BRepAdaptor_Curve.hxx>
#include <Precision.hxx>
#include <BRep_Tool.hxx>
#include <TopoDS.hxx>
#include <GeomAPI_ProjectPointOnSurf.hxx>
#include <BRepClass3d_SolidClassifier.hxx>
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
            if (!faceShape.isNull()) {
                Part::TopoShape tempShape = faceShape.makeElementFace(nullptr, "Part::FaceMakerCheese");
                GProp_GProps props;
                BRepGProp::SurfaceProperties(tempShape.getSubTopoShape(TopAbs_FACE, 1).getShape(), props);
              
                gp_Pnt cog = props.CentreOfMass();
                mBehaviour->setUpOriginPos(cog.X(),cog.Y(),cog.Z());
            }
         
           
        }
        void setAxis(const gp_Ax1& ax) {
            axis = ax;
            
            mBehaviour->setUpRotateAxis(
                ax.Direction().X(), ax.Direction().Y(), ax.Direction().Z());
            mBehaviour->setUpRotateCenter(ax.Location().X(),
                ax.Location().Y(), ax.Location().Z());
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
            SelectionManager::instance().RemoveObserver(selectEdgeObserver.tag);
            delete selectEdgeObserver.command;
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
        bool  reverse;
       
        ExecuteCommandPair selectEdgeObserver;
    };

    RevolutionTask::RevolutionTask(RevolutionType type,QWidget* parent)
        : ParamTaskDialog(parent),mInternal(new Internal(this))
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
        mInternal->angleProp->setMinMax(0,360);

        addParam(mInternal->angleProp);
        EnumProperty* join = new EnumProperty("Axis", p);
        addParam(join);
        BoolProperty* reverse = new BoolProperty("Reverse", p);
        addParam(reverse);
        buildUi();
        mInternal->selectEdgeObserver=SelectionManager::instance().AddObserver(SelectAny, this, &RevolutionTask::onSelectAny);
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
        bool updatePreView = false;
        if (propertyName == "Revolve:Angle") {
            mInternal->angle = value.toFloat();
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
        if (mInternal->faceShape.isNull()) {
            return false;
        }
        gp_Ax1 raxis=mInternal->axis;
        if (mInternal->reverse) {
            raxis.Reverse();
        }
        float radAngle = mInternal->angle * 3.14159265358979323846f / 180.0f;
        Part::TopoShape revolve= mInternal->faceShape.makeElementRevolve(raxis, radAngle, "Part::FaceMakerBullseye");
        getPreviewShape() = revolve;
        Part::TopoShape resShape;
        if (!mInternal->baseShape.isNull()) {
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
    void RevolutionTask::onSelectAny()
    {
        if (mInternal->axisType == 2) {
            CORE_INFO("Any thing selected");
            std::vector<Part::TopoShape>shapes;
            ViewTool::getSelectedTopoShape(shapes);
            if (shapes.size() > 0) {
                if (shapes[1].getShape().ShapeType() == TopAbs_ShapeEnum::TopAbs_EDGE) {
                    TopoDS_Edge refEdge = TopoDS::Edge(shapes[1].getShape());
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
                    mInternal->setAxis(gp_Ax1 (b, d));
                    previewShape();
                }
                else
                {
                    CORE_ERROR("not a edge to use as axis");
                }
            }
        }
    }
}