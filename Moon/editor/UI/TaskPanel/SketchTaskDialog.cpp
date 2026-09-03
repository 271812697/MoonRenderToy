#include "SketchTaskDialog.h"
#include "TaskBox.h"
#include "Sketcher/SketcherObjManager.h"
#include "Sketcher/SketcherObj.h"
#include "feature/SketcherFeature.h"
#include "Widgets/BoolProperty.h"
#include "core/ViewTool.h"
#include "Interactive/Widgets/SketchPlane.h"
#include <QLabel>
#include <QLineEdit>
#include <QCheckBox>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QGroupBox>
#include <gp_Pln.hxx>
namespace MOON {

    class SketchTaskDialog::Internal
    {
    public:
        Internal(SketchTaskDialog* s) :self(s) {
            auto f = self->getFeature();
            if (f) {
                //
				feature = dynamic_cast<SketcherFeature*>(f);
                feature->getSketcherObj()->beginEdit();

            }
            else
            {
                feature = SketcherObjManager::instance().CreateSketcherFeature();
                //feature->getSketcherObj()->setActive(true);
				self->setFeature(feature);
                Feature* baseFeature = nullptr;
                std::vector<std::string>subValues;
                ViewTool::getSelectedBasedFeature(baseFeature, subValues);
                if (baseFeature) {
                    feature->setBaseFeature(baseFeature);
                    feature->setSubValues(subValues);
                    Part::TopoShape face = feature->getBaseTopoFaceShape();
                    gp_Pln pln;
                    face.findPlane(pln);
                    SketcherPlane2D plane;
                    plane.normal = Base::Vector3d{ pln.Axis().Direction().X(),pln.Axis().Direction().Y(),pln.Axis().Direction().Z() };
                    plane.origin = Base::Vector3d{ pln.Location().X(),pln.Location().Y(),pln.Location().Z() };
                    plane.xAxis = Base::Vector3d{ pln.XAxis().Direction().X(),pln.XAxis().Direction().Y(),pln.XAxis().Direction().Z() };
                    plane.yAxis = Base::Vector3d{ pln.YAxis().Direction().X(),pln.YAxis().Direction().Y(),pln.YAxis().Direction().Z() };
                    feature->getSketcherObj()->setPlane(plane);
                }
                else
                {
					behaviour = new SketchPlane("selectPlane");
                    behaviour->AddObserver(SketchPlaneEvent::SelectPlane,self, &SketchTaskDialog::onSelectPlane);
                }
            }
            SketcherObjManager::instance().setCurrentActiveSketcherFeature(feature);
        }
        ~Internal() {
            feature->getSketcherObj()->setActive(false);
            if (behaviour) {
                delete behaviour;
            }
        }
    private:
        friend SketchTaskDialog;
        SketchTaskDialog* self = nullptr;
        SketcherFeature* feature = nullptr;
        SketchPlane* behaviour = nullptr;
    };

    SketchTaskDialog::SketchTaskDialog(QWidget* parent, Feature* feature)
        : ParamTaskDialog(parent),mInternal(new Internal(this)),ShapeHelper(feature)
    {
        PropertyComponent* sketchGroup = addGroupParam("Sketch");
        addParam(new BoolProperty("Draw Grid", sketchGroup));
        buildUi();
    }

    SketchTaskDialog::~SketchTaskDialog()
    {
        delete mInternal;
    }

    QVariant SketchTaskDialog::getParamValue(const QString& propertyName)
    {
        if (propertyName == "Sketch:Draw Grid" && mInternal->feature) {
            return QVariant::fromValue(mInternal->feature->getSketcherObj()->isDrawGrid());
        }
        return QVariant();
    }

    void SketchTaskDialog::setParamValue(const QString& propertyName, const QVariant& value)
    {
        if (propertyName == "Sketch:Draw Grid" && mInternal->feature) {
            mInternal->feature->getSketcherObj()->setDrawGrid(value.value<bool>());
        }
    }

    
    void SketchTaskDialog::clickOk()
    {
        mInternal->feature->getSketcherObj()->makeDone();
        mInternal->feature->execute();
        mInternal->feature->makeDone();
    }
    void SketchTaskDialog::clickApply()
    {
    }
    void SketchTaskDialog::clickCancel()
    {
    }
    void SketchTaskDialog::onSelectPlane()
    {
        if (mInternal->behaviour) {
            mInternal->feature->getSketcherObj()->setPlane(mInternal->behaviour->getSelectPlane());
        }
    }
}
