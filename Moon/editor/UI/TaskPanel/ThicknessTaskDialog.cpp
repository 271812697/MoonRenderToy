#include "editor/UI/TaskPanel/ThicknessTaskDialog.h"
#include "TaskBox.h"
#include "Widgets/SliderFloatProperty.h"
#include "Widgets/EnumProperty.h"
#include "Widgets/BoolProperty.h"
#include "TopoShape.h"
#include "core/ViewTool.h"
#include "core/log.h"
#include "Interactive/Widgets/AxisTranslationWidget.h"
#include "Geometry.h"

#include <gp_Pln.hxx>
namespace MOON {

    class ThicknessTaskDialog::Internal {
    public:
        Internal(ThicknessTaskDialog*s):self(s){
            axisBehaviour = new AxisTranslationWidget("thickness");
            ViewTool::getSelectedTopoShape(shapes);
            if (shapes.size() > 0) {
             
                Part::TopoShape outWire=shapes[1].splitWires();
                //outWire.isLinearEdge
                Part::TopoShape edge=outWire.getOrderedEdges().front();
                std::unique_ptr<Part::Geometry>curve=Part::Geometry::fromShape(edge.getShape());
                Part::GeomCurve* geoCurve=dynamic_cast<Part::GeomCurve*>(curve.get());
                double u0 = geoCurve->getFirstParameter();
                double u1 = geoCurve->getLastParameter();
                double u=(u0+u1)*0.5;
               
                Base::Vector3d origin = geoCurve->value(u);
                dir=geoCurve->firstDerivativeAtParameter(u);
                gp_Pln pln;
                shapes[1].findPlane(pln);
                Base::Vector3d nor{ pln.Axis().Direction().X(), pln.Axis().Direction().Y(), pln.Axis().Direction().Z() };
                dir = dir.Cross(nor);
                axisBehaviour->setUpOrigin(origin.x,origin.y,origin.z); 
                if (reverse) {
                    axisBehaviour->setUpDir(-dir.x,-dir.y,-dir.z);
                }
                else {
                    axisBehaviour->setUpDir(dir.x, dir.y, dir.z);
                }
               
                axisBehaviour->setImmediateInvoke(false);
                axisBehaviour->setLength(thickNessValue);
                axisBehaviour->AddObserver(AxisTranslationEvent::LengthChange, self, &ThicknessTaskDialog::onWidgetLengthInvoke);

            }
        }
        ~Internal() {
            delete axisBehaviour;
        }
    private:
        SliderFloatProperty* thickNessProp;
        friend ThicknessTaskDialog;
        ThicknessTaskDialog* self = nullptr;
        float thickNessValue = 0.5;
        int mode = 0;
        int joinType = 0;
        bool reverse = false;
        bool intersection = false;
        Base::Vector3d dir;
        AxisTranslationWidget* axisBehaviour = nullptr;
       
        std::vector<Part::TopoShape>shapes;
    };

    ThicknessTaskDialog::ThicknessTaskDialog(QWidget* parent)
        : ParamTaskDialog(parent),mInternal(new Internal(this))
    {
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
            return QVariant::fromValue(mInternal->thickNessValue);
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
            return QVariant::fromValue(mInternal->reverse);
        }
        else if (propertyName == "Thickness:Intersection") {
            return QVariant::fromValue(mInternal->intersection);
        }
        return QVariant();
    }

    void ThicknessTaskDialog::setParamValue(const QString& propertyName, const QVariant& value)
    {
        bool updatePreView = false;
        if (propertyName == "Thickness:Thickness value") {
            mInternal->thickNessValue=value.toFloat();
            mInternal->axisBehaviour->setLength(mInternal->thickNessValue);
            updatePreView = true;
        }
        else if (propertyName == "Thickness:Mode") {
            mInternal->mode = value.value<int>();
            updatePreView = true;
        }
        else if (propertyName == "Thickness:Join Type") {
            mInternal->joinType = value.value<int>();
            updatePreView = true;
        }
        else if (propertyName == "Thickness:Reverse") {
            mInternal->reverse = value.value<bool>();
            if (mInternal->reverse) {
                mInternal->axisBehaviour->setUpDir(-mInternal->dir.x, -mInternal->dir.y, -mInternal->dir.z);
            }
            else {
                mInternal->axisBehaviour->setUpDir(mInternal->dir.x, mInternal->dir.y, mInternal->dir.z);
            }
            mInternal->axisBehaviour->setLength(mInternal->thickNessValue);
            updatePreView = true;
        }
        else if (propertyName == "Thickness:Intersection") {
            mInternal->intersection = value.value<bool>();
            updatePreView = true;
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
    }
    bool ThicknessTaskDialog::generateShape()
    {
        if (mInternal->shapes.size() > 0) {
            double tol = Precision::Confusion();
            double thickness = (mInternal->reverse ? -1. : 1.) * mInternal->thickNessValue;
            int join = mInternal->joinType;
            if (join == 1) {
                join = 2;
            }

            if (fabs(thickness) > 2 * tol) {
                try
                {
                    Part::TopoShape shape=mInternal->shapes[0].makeElementThickSolid({ mInternal->shapes[1] }, thickness, tol, mInternal->intersection, false, mInternal->mode, static_cast<Part::JoinType>(join));
                    getPreviewShape() = shape;
                    getGenerateShape() = shape;
                    return true;
                }
                catch (Standard_Failure& e)
                {
                    CORE_ERROR(e.GetMessageString());
                    return false;
                }
                return false;
            }
        }
        return false;
    }
    void ThicknessTaskDialog::onWidgetLengthInvoke() {
        mInternal->thickNessValue = mInternal->axisBehaviour->getLength();
        mInternal->thickNessProp->updateWidgetValue(mInternal->thickNessValue);
        previewShape();
    }
}