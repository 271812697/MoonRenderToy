#include "editor/UI/TaskPanel/FilletTask.h"
#include "TaskBox.h"
#include "Widgets/SliderFloatProperty.h"
#include "Widgets/EnumProperty.h"
#include "Widgets/BoolProperty.h"
#include "TopoShape.h"
#include "core/ViewTool.h"
#include "core/log.h"
#include "Interactive/Widgets/AxisTranslationWidget.h"
#include "Geometry.h"


#include <ShapeFix_ShapeTolerance.hxx>
#include <BRepAlgo.hxx>
namespace MOON {

    class FilletTask::Internal {
    public:
        Internal(FilletTask*s):self(s){
            axisBehaviour = new AxisTranslationWidget("fillet");
            ViewTool::getSelectedTopoShape(shapes);
            if (shapes.size() > 0) {
                axisBehaviour->setImmediateInvoke(false);
                axisBehaviour->setLength(radius);
                //axisBehaviour->AddObserver(AxisTranslationEvent::LengthChange, self, &FilletTask::onWidgetLengthInvoke);
            }
        }
        ~Internal() {
            delete axisBehaviour;
        }
    private:
        SliderFloatProperty* radiusProp;
        friend FilletTask;
        FilletTask* self = nullptr;
        float radius = 0.5;
        bool useAllEdges = false;
       
        AxisTranslationWidget* axisBehaviour = nullptr;
       
        std::vector<Part::TopoShape>shapes;
    };

    FilletTask::FilletTask(QWidget* parent)
        : ParamTaskDialog(parent),mInternal(new Internal(this))
    {       
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
            mInternal->axisBehaviour->setLength(mInternal->radius);
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
    void FilletTask::onWidgetLengthInvoke() {
        mInternal->radius= mInternal->axisBehaviour->getLength();
        mInternal->radiusProp->updateWidgetValue(mInternal->radius);
        previewShape();
    }
}