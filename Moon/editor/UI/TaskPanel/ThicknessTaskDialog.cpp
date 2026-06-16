#include "editor/UI/TaskPanel/ThicknessTaskDialog.h"
#include "TaskBox.h"
#include "Widgets/SliderFloatProperty.h"
#include "Widgets/EnumProperty.h"
#include "Widgets/BoolProperty.h"
#include "TopoShape.h"
#include "core/ViewTool.h"
#include "core/log.h"
namespace MOON {

    class ThicknessTaskDialog::Internal {
    public:
        Internal(ThicknessTaskDialog*s):self(s){}
        ~Internal() {
        }
    private:
        friend ThicknessTaskDialog;
        ThicknessTaskDialog* self = nullptr;
        float thickNessValue = 0.5;
        int mode = 0;
        int joinType = 0;
        bool reverse = false;
        bool intersection = false;
    };

    ThicknessTaskDialog::ThicknessTaskDialog(QWidget* parent)
        : ParamTaskDialog(parent),mInternal(new Internal(this))
    {
        PropertyComponent* p=addGroupParam("Thickness");
        SliderFloatProperty* thickNessValue = new SliderFloatProperty("Thickness value", p);
        thickNessValue->setMinMax(0.1,10);
        addParam( thickNessValue);
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
    bool ThicknessTaskDialog::generatePreviewShape()
    {
        std::vector<Part::TopoShape>shapes;
        if (ViewTool::getSelectedTopoShape(shapes)) {
            double tol = Precision::Confusion();
            double thickness = (mInternal->reverse ? -1. : 1.) * mInternal->thickNessValue;
            int join = mInternal->joinType;
            if (join == 1) {
                join = 2;
            }

            if (fabs(thickness) > 2 * tol) {
                try
                {
                    //CORE_INFO("the num of solid is {}", shapes[0].countSubShapes(TopAbs_SOLID));
                    getPreviewShape() = shapes[0].makeElementThickSolid({ shapes[1] }, thickness, tol, mInternal->intersection, false, mInternal->mode, static_cast<Part::JoinType>(join));
                    return true;
                }
                catch (Standard_Failure& e)
                {
                    CORE_ERROR(e.GetMessageString());
                    return false;
                }
                //ViewTool::createTopoActor(res,"ThickNessTopoShape"); 
                return false;
            }
        }
        return false;
    }
}