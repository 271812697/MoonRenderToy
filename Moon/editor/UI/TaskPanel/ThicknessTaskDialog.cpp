#include "editor/UI/TaskPanel/ThicknessTaskDialog.h"
#include "Widgets/SliderFloatProperty.h"
#include "Widgets/EnumProperty.h"
#include "Widgets/BoolProperty.h"
#include "TopoShape.h"
#include "core/ViewTool.h"
#include "core/log.h"
#include "Interactive/Widgets/AxisTranslationWidget.h"
#include "feature/ThicknessFeature.h"
#include "App/GizmoHelper.h"
#include "base/BoundBox.h"
#include <GeomAbs_Shape.hxx>
#include <BRep_Tool.hxx>
#include <TopoDS.hxx>
#include <TopExp_Explorer.hxx>
namespace MOON {
    std::vector<Part::TopoShape> getContinuousEdges(const Part::TopoShape& shape,const Part::TopoShape& face) {
        std::vector<Part::TopoShape> ret;
        auto addEdge = [&](const TopoDS_Shape& subShape) {
            auto faces = shape.findAncestorsShapes(subShape, TopAbs_FACE);
            if (faces.size() != 2) {
                CORE_WARN(": skip edge ");
                return;
            }
            const TopoDS_Shape& face1 = faces.front();
            const TopoDS_Shape& face2 = faces.back();
            GeomAbs_Shape cont
                = BRep_Tool::Continuity(TopoDS::Edge(subShape), TopoDS::Face(face1), TopoDS::Face(face2));
            if (cont != GeomAbs_C0) {
                CORE_WARN( ": skip edge that is not C0 continuous");
                return;
            }
            ret.push_back(subShape);
        };
        for (TopExp_Explorer exp(face.getShape(), TopAbs_EDGE); exp.More(); exp.Next()) {
            addEdge(exp.Current());
        }
        return ret;
    }
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
                    std::vector<Part::TopoShape>shapes=getContinuousEdges(baseShape,baseFace);
                    Part::TopoShape edge = shapes[0];
                    DraggerPlacementProps props = getDraggerPlacementFromEdgeAndFace(edge, baseFace);
                    feature->dir[0] = -props.dir.x;
                    feature->dir[1] = -props.dir.y;
                    feature->dir[2] = -props.dir.z;
                    feature->midPoint[0] = props.position.x;
                    feature->midPoint[1] = props.position.y;
                    feature->midPoint[2] = props.position.z;
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
        return false;
    }
    void ThicknessTaskDialog::onWidgetLengthInvoke() {
        mInternal->feature->thickNessValue = mInternal->axisBehaviour->getLength();
        mInternal->thickNessProp->updateWidgetValue(mInternal->feature->thickNessValue);
    }
}