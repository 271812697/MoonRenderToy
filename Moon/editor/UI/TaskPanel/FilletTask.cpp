#include "editor/UI/TaskPanel/FilletTask.h"
#include "TaskBox.h"
#include "Widgets/SliderFloatProperty.h"
#include "Widgets/EnumProperty.h"
#include "Widgets/BoolProperty.h"
#include "TopoShape.h"
#include "core/ViewTool.h"
#include "core/log.h"
#include "Interactive/Widgets/AxisTranslationWidget.h"
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
namespace MOON {

    class FilletTask::Internal {
    public:
        Internal(FilletTask*s):self(s){
   
            ViewTool::getSelectedTopoShape(shapes);
            if (shapes.size() > 0) {
                if (shapes[1].getShape().ShapeType() != TopAbs_ShapeEnum::TopAbs_EDGE) {
                    CORE_ERROR("is not a Edge to fillet");
                    return;
                }
                auto len = shapes[0].getBoundBoxOptimal().CalcDiagonalLength()*0.01;;
                radius = shapes[0].getBoundBoxOptimal().CalcDiagonalLength() * 0.03;

                axisBehaviour1 = new AxisTranslationWidget("fillet");
                axisBehaviour2 = new AxisTranslationWidget("fillet");
                axisBehaviour1->setUpScale(len);
                axisBehaviour2->setUpScale(len);
                // 1. 获取边
                TopoDS_Edge edge = TopoDS::Edge(shapes[1].getShape());

                // 2. 获取边的中点坐标和切向量
                double first, last;
                Handle(Geom_Curve) curve = BRep_Tool::Curve(edge, first, last);
                double midParam = (first + last) * 0.5;
                gp_Pnt midPoint;
                gp_Vec tangent;
                curve->D1(midParam, midPoint, tangent);
                tangent.Normalize();

                // 3. 获取两个面
                auto faces = shapes[0].findAncestorsShapes(shapes[1].getShape(), TopAbs_FACE);
                auto solids = shapes[0].findAncestorsShapes(shapes[1].getShape(), TopAbs_SOLID);
                if (faces.size() != 2) {
                    CORE_ERROR("Edge must be shared by exactly two faces");
                    return;
                }
                TopoDS_Face face1 = TopoDS::Face(faces[0]);
                TopoDS_Face face2 = TopoDS::Face(faces[1]);
                TopoDS_Solid solid = TopoDS::Solid(solids[0]);

                // 4. 计算某个面上、过边上一点、位于面内且垂直于边的方向
                auto getInPlanePerpDir = [&](const TopoDS_Solid& solid,const TopoDS_Face& face, const gp_Pnt& point, const gp_Vec& tangent) -> gp_Vec {
                    BRepAdaptor_Surface surf(face);

                    gp_Vec normal;
                    if (surf.GetType() == GeomAbs_Plane) {
                        // 平面：直接用平面法向
                        normal = surf.Plane().Axis().Direction();
                    }
                    else {
                        // 曲面：将点投影到曲面，获取 uv 参数后计算法向
                        Handle(Geom_Surface) surface = BRep_Tool::Surface(face);
                        GeomAPI_ProjectPointOnSurf projector(point, surface);
                        if (projector.NbPoints() == 0) {
                            CORE_ERROR("Failed to project point onto surface");
                            return gp_Vec(0, 0, 1); // 默认方向
                        }
                        double u, v;
                        projector.Parameters(1, u, v);
                        gp_Pnt P;
                        gp_Vec dU, dV;
                        surf.D1(u, v, P, dU, dV);
                        normal = dU.Crossed(dV);
                    }
                    normal.Normalize();
                    // 面内垂直于边的方向 = 切向量 × 法向量
                    gp_Vec dir = tangent.Crossed(normal);
                    dir.Normalize();
                    // 沿 dir 方向偏移一点
                    gp_Pnt testPoint = point.XYZ() + dir.XYZ() * 0.001;
                    // 判断 testPoint 是否在实体内部
                    BRepClass3d_SolidClassifier classifier(solid);
                    classifier.Perform(testPoint, Precision::Confusion());
                    if (classifier.State() == TopAbs_OUT) {
                        // 如果在外部，反转方向
                        dir.Reverse();
                    }
                    return dir;
                    };

                gp_Vec dir1 = getInPlanePerpDir(solid,face1, midPoint, tangent);
                gp_Vec dir2 = getInPlanePerpDir(solid,face2, midPoint, tangent);


                axisBehaviour1->setImmediateInvoke(false);
                axisBehaviour1->setLength(radius);
                axisBehaviour1->setUpOrigin(midPoint.X(), midPoint.Y(), midPoint.Z());
                axisBehaviour1->setUpDir(dir1.X(), dir1.Y(), dir1.Z());
                axisBehaviour2->setImmediateInvoke(false);
                axisBehaviour2->setLength(radius);
                axisBehaviour2->setUpOrigin(midPoint.X(), midPoint.Y(), midPoint.Z());
                axisBehaviour2->setUpDir(dir2.X(), dir2.Y(), dir2.Z());
                axisBehaviour1->AddObserver(AxisTranslationEvent::LengthChange, self, &FilletTask::onWidgetLengthInvoke1);
                axisBehaviour2->AddObserver(AxisTranslationEvent::LengthChange, self, &FilletTask::onWidgetLengthInvoke2);
            }
        }
        ~Internal() {
            if (axisBehaviour1) {
                delete axisBehaviour1;
                delete axisBehaviour2;
            }

        }
    private:
        SliderFloatProperty* radiusProp;
        friend FilletTask;
        FilletTask* self = nullptr;
        float radius = 0.5;
        bool useAllEdges = false;
       
        AxisTranslationWidget* axisBehaviour1 = nullptr;
        AxisTranslationWidget* axisBehaviour2 = nullptr;
       
        std::vector<Part::TopoShape>shapes;
    };

    FilletTask::FilletTask(QWidget* parent)
        : ParamTaskDialog(parent),mInternal(new Internal(this))
    {       
        setGenerateShapeName("FilletShape");
        mPreviewOption.isTransparent = false;
        mPreviewOption.isBlend = true;
        mPreviewOption.useDomainColor = false;
        PropertyComponent* p=addGroupParam("Fillet");
        mInternal->radiusProp = new SliderFloatProperty("Radius", p);
        mInternal->radiusProp->setMinMax(0.1,10);
        addParam(mInternal->radiusProp);
        BoolProperty* intersection = new BoolProperty("Use ALL Edges", p);
        addParam(intersection);
        buildUi();
    }

    FilletTask::~FilletTask()
    {
        delete mInternal;
    }

    QVariant FilletTask::getParamValue(const QString& propertyName)
    {
        if (propertyName == "Fillet:Radius") {
            return QVariant::fromValue(mInternal->radius);
        }
        else if (propertyName == "Use ALL Edges") {
            return QVariant::fromValue(mInternal->useAllEdges);
        }
        return QVariant();
    }

    void FilletTask::setParamValue(const QString& propertyName, const QVariant& value)
    {
        bool updatePreView = false;
        if (propertyName == "Fillet:Radius") {
            mInternal->radius=value.toFloat();
            mInternal->axisBehaviour1->setLength(mInternal->radius);
            mInternal->axisBehaviour2->setLength(mInternal->radius);
            updatePreView = true;
        }
       
        else if (propertyName == "Use ALL Edges") {
            mInternal->useAllEdges= value.value<bool>();
            updatePreView = true;
        }
        if (updatePreView) {
            previewShape();
        }
    }


    void FilletTask::clickOk()
    {
        generateFinalShape();
    }
    void FilletTask::clickApply()
    {
    }
    void FilletTask::clickCancel()
    {
    }
    bool FilletTask::generateShape()
    {
        if (mInternal->shapes.size() > 0) {
           
            std::vector<Part::TopoShape> edges=mInternal->useAllEdges ? mInternal->shapes[0].getSubTopoShapes(TopAbs_EDGE) : std::vector<Part::TopoShape>{ mInternal->shapes[1].getShape() };
            try
            {
                Part::TopoShape resShape(0);
                resShape.makeElementFillet(mInternal->shapes[0],edges,mInternal->radius, mInternal->radius);
                TopTools_ListOfShape aLarg;
                aLarg.Append(mInternal->shapes[0].getShape());
                if (!BRepAlgo::IsValid(aLarg, mInternal->shapes[0].getShape(), Standard_False, Standard_False)) {
                    ShapeFix_ShapeTolerance aSFT;
                    aSFT.LimitTolerance(
                        mInternal->shapes[0].getShape(),
                        Precision::Confusion(),
                        Precision::Confusion(),
                        TopAbs_SHAPE
                    );
                }
                getGenerateShape() = resShape;
                getPreviewShape() = mInternal->shapes[0].makeElementCut(resShape);
                return true;
            }
            catch (Standard_Failure& e)
            {
                CORE_ERROR(e.GetMessageString());
                return false;
            }
            return true;
        
        }
        return false;
    }
    void FilletTask::onWidgetLengthInvoke1()
    {        
        mInternal->radius= mInternal->axisBehaviour1->getLength();
        mInternal->radiusProp->updateWidgetValue(mInternal->radius);
        previewShape();
    }
    void FilletTask::onWidgetLengthInvoke2() {
        mInternal->radius = mInternal->axisBehaviour2->getLength();
        mInternal->radiusProp->updateWidgetValue(mInternal->radius);
        previewShape();
    }
}