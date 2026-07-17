#include "editor/UI/TaskPanel/ThicknessTaskDialog.h"
#include "TaskBox.h"
#include "Widgets/SliderFloatProperty.h"
#include "Widgets/EnumProperty.h"
#include "Widgets/BoolProperty.h"
#include "TopoShape.h"
#include "core/ViewTool.h"
#include "core/log.h"
#include "Interactive/Widgets/AxisTranslationWidget.h"
#include "feature/ThicknessFeature.h"
#include "Geometry.h"
#include "base/BoundBox.h"
#include <gp_Pln.hxx>
#include <BRepTools.hxx>
#include <BRepAdaptor_Surface.hxx>
#include <gp_Vec.hxx>
#include <gp_Dir.hxx>
#include <GeomAbs_Shape.hxx>
#include <ShapeFix_ShapeTolerance.hxx>
#include <BRepAlgo.hxx>
#include <ShapeAnalysis_Surface.hxx>
#include <BRepLProp_SLProps.hxx>
#include <Precision.hxx>
#include <BRepAdaptor_Curve.hxx>
#include <BRep_Tool.hxx>
#include <TopoDS.hxx>
#include <GeomAPI_ProjectPointOnSurf.hxx>
#include <BRepClass3d_SolidClassifier.hxx>
namespace MOON {

    class ThicknessTaskDialog::Internal {
    public:
        Internal(ThicknessTaskDialog*s):self(s){
            auto f = self->getFeature();
			if (f) {
				feature = dynamic_cast<ThicknessFeature*>(f);
            }
            else
            {
                Feature* baseFeature = nullptr;
                std::vector<std::string>subValues;
                ViewTool::getSelectedBasedFeature(baseFeature, subValues);
                if (baseFeature) {
					isCreatedFeature = true;
					feature = new ThicknessFeature("Thickness");
					feature->setBaseFeature(baseFeature);
					feature->setSubValues(subValues);
					self->setFeature(feature);

                    Part::TopoShape baseShape = feature->getBaseTopoShape();
                    Part::TopoShape baseFace = feature->getBaseTopoFaceShape();

                    double boxLen = baseFace.getBoundBoxOptimal().CalcDiagonalLength();
                    feature->scale = boxLen * 0.01;;

                    feature->thickNessValue = boxLen * 0.02;
                    Part::TopoShape outWire = baseFace.splitWires();
                    //outWire.isLinearEdge
                    auto solids = baseShape.findAncestorsShapes(baseFace.getShape(), TopAbs_SOLID);
                    TopoDS_Edge edge = TopoDS::Edge(outWire.getOrderedEdges().front().getShape());
                    TopoDS_Solid solid = TopoDS::Solid(solids[0]);
                    // 2. 获取边的中点坐标和切向量
                    double first, last;
                    Handle(Geom_Curve) curve = BRep_Tool::Curve(edge, first, last);
                    double midParam = (first + last) * 0.5;
                    gp_Pnt midPoint;
                    gp_Vec tangent;
                    curve->D1(midParam, midPoint, tangent);
                    tangent.Normalize();
                    TopoDS_Face face = TopoDS::Face(baseFace.getShape());
                    // 4. 计算某个面上、过边上一点、位于面内且垂直于边的方向
                    auto getInPlanePerpDir = [&](const TopoDS_Solid& solid, const TopoDS_Face& face, const gp_Pnt& point, const gp_Vec& tangent) -> gp_Vec {
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
                    gp_Vec d = getInPlanePerpDir(solid, face, midPoint, tangent);

                    feature->dir[0] = -d.X();
                    feature->dir[1] = -d.Y();
                    feature->dir[2] = -d.Z();
                    feature->midPoint[0] = midPoint.X();
                    feature->midPoint[1] = midPoint.Y();
                    feature->midPoint[2] = midPoint.Z();
                }
            }
            if (feature) {  
       
                //axisBehaviour->setImmediateInvoke(false);
                axisBehaviour = new AxisTranslationWidget("thickness");
                axisBehaviour->setUpOrigin(feature->midPoint[0], feature->midPoint[1], feature->midPoint[2]);
                if (feature->reverse) {
                    axisBehaviour->setUpDir(-feature->dir[0], -feature->dir[1], -feature->dir[2]);
                }
                else {
                    axisBehaviour->setUpDir(feature->dir[0], feature->dir[1], feature->dir[2]);
                }
                axisBehaviour->setLength(feature->thickNessValue);
                axisBehaviour->AddObserver(AxisTranslationEvent::LengthChange, self, &ThicknessTaskDialog::onWidgetLengthInvoke);
                axisBehaviour->setUpScale(feature->scale);
            }
        }
        ~Internal() {
            if (axisBehaviour) {
                delete axisBehaviour;
            }
        }
    private:
        SliderFloatProperty* thickNessProp;
        friend ThicknessTaskDialog;
        ThicknessTaskDialog* self = nullptr;
		ThicknessFeature* feature = nullptr;
        AxisTranslationWidget* axisBehaviour = nullptr;
        bool isCreatedFeature = false;
    };

    ThicknessTaskDialog::ThicknessTaskDialog(QWidget* parent, Feature* feature)
        : ParamTaskDialog(parent),ShapeHelper(feature), mInternal(new Internal(this))
    {
        setGenerateShapeName("ThickShape");
        PropertyComponent* p=addGroupParam("Thickness");
        mInternal->thickNessProp = new SliderFloatProperty("Thickness value", p);
        mInternal->thickNessProp->setMinMax(0.1,10);
        addParam(mInternal->thickNessProp);
        EnumProperty* mode=new EnumProperty("Mode", p);
        addParam(mode);
        EnumProperty* join = new EnumProperty("Join Type", p);
        addParam(join);
        BoolProperty* intersection = new BoolProperty("Intersection", p);
        addParam(intersection);
        BoolProperty* reverse=new BoolProperty("Reverse", p);
        addParam(reverse);
        buildUi();
    }

    ThicknessTaskDialog::~ThicknessTaskDialog()
    {
        delete mInternal;
    }

    QVariant ThicknessTaskDialog::getParamValue(const QString& propertyName)
    {
        if (propertyName == "Thickness:Thickness value") {
            return QVariant::fromValue(mInternal->feature->thickNessValue);
        }
        else if (propertyName == "Thickness:Mode") {
            QList<QString>list = { "Skin", "Pipe", "RectoVerso" };
            return QVariant::fromValue(list);
        }
        else if (propertyName == "Thickness:Join Type") {
            QList<QString>list = { "Arc", "Intersection" };
            return QVariant::fromValue(list);
        }
        else if (propertyName == "Thickness:Reverse") {
            return QVariant::fromValue(mInternal->feature->reverse);
        }
        else if (propertyName == "Thickness:Intersection") {
            return QVariant::fromValue(mInternal->feature->intersection);
        }
        return QVariant();
    }

    void ThicknessTaskDialog::setParamValue(const QString& propertyName, const QVariant& value)
    {
        bool updatePreView = false;
        if (propertyName == "Thickness:Thickness value") {
            mInternal->feature->thickNessValue=value.toFloat();
            if (mInternal->axisBehaviour) {
                mInternal->axisBehaviour->setLength(mInternal->feature->thickNessValue);
                updatePreView = true;
            }

        }
        else if (propertyName == "Thickness:Mode") {
            mInternal->feature->mode = value.value<int>();
            if (mInternal->axisBehaviour) {
                updatePreView = true;
            }

        }
        else if (propertyName == "Thickness:Join Type") {
            mInternal->feature->joinType = value.value<int>();
            if (mInternal->axisBehaviour) {
                 updatePreView = true;
            }
        }
        else if (propertyName == "Thickness:Reverse") {
            mInternal->feature->reverse = value.value<bool>();
            if (mInternal->axisBehaviour) {
                if (mInternal->feature->reverse) {
                    mInternal->axisBehaviour->setUpDir(-mInternal->feature->dir[0], -mInternal->feature->dir[1], -mInternal->feature->dir[2]);
                }
                else {
                    mInternal->axisBehaviour->setUpDir(mInternal->feature->dir[0], mInternal->feature->dir[1], mInternal->feature->dir[2]);
                }
                mInternal->axisBehaviour->setLength(mInternal->feature->thickNessValue);
                updatePreView = true;
            }
        }
        else if (propertyName == "Thickness:Intersection") {
            mInternal->feature->intersection = value.value<bool>();
            if (mInternal->axisBehaviour) {
                updatePreView = true;
            }
        }
        if (updatePreView) {
            previewShape();
        }
    }


    void ThicknessTaskDialog::clickOk()
    {
        generateFinalShape();
    }
    void ThicknessTaskDialog::clickApply()
    {
    }
    void ThicknessTaskDialog::clickCancel()
    {
        clearPreviewShape();

        if (mInternal->isCreatedFeature) {
            mInternal->feature->RemoveFromScene();
            delete mInternal->feature;
        }
    }
    bool ThicknessTaskDialog::generateShape()
    {
        //if (mInternal->shapes.size() > 0) {
        //    double tol = Precision::Confusion();
        //    double thickness = (mInternal->feature->reverse ? -1. : 1.) * mInternal->feature->thickNessValue;
        //    int join = mInternal->feature->joinType;
        //    if (join == 1) {
        //        join = 2;
        //    }

        //    if (fabs(thickness) > 2 * tol) {
        //        try
        //        {
        //            Part::TopoShape shape=mInternal->shapes[0].makeElementThickSolid({ mInternal->shapes[1] }, thickness, tol, mInternal->intersection, false, mInternal->mode, static_cast<Part::JoinType>(join));
        //            getPreviewShape() = shape;
        //            getGenerateShape() = shape;
        //            return true;
        //        }
        //        catch (Standard_Failure& e)
        //        {
        //            CORE_ERROR(e.GetMessageString());
        //            return false;
        //        }
        //        return false;
        //    }
        //}
        return false;
    }
    void ThicknessTaskDialog::onWidgetLengthInvoke() {
        mInternal->feature->thickNessValue = mInternal->axisBehaviour->getLength();
        mInternal->thickNessProp->updateWidgetValue(mInternal->feature->thickNessValue);
    }
}