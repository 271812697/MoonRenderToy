#include "SketchTaskDialog.h"
#include "TaskBox.h"
#include "editor/UI/PropertyPanel/Collapsiblegroupboxwidget.h"
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
#include <QListWidget>
#include <QStringList>
#include <QTimer>
#include <QIcon>
#include <cstdio>
namespace MOON {
	static QString constraintIconPath(Sketcher::ConstraintType type)
	{
		const QString prefix = ":/widgets/icons/constraint/";
		switch (type) {
		case Sketcher::ConstraintType::Coincident:
			return prefix + "Constraint_Coincident.svg";
		case Sketcher::ConstraintType::Horizontal:
			return prefix + "Constraint_Horizontal.svg";
		case Sketcher::ConstraintType::Vertical:
			return prefix + "Constraint_Vertical.svg";
		case Sketcher::ConstraintType::Parallel:
			return prefix + "Constraint_Parallel.svg";
		case Sketcher::ConstraintType::Tangent:
			return prefix + "Constraint_Tangent.svg";
		case Sketcher::ConstraintType::Distance:
			return prefix + "Constraint_Length.svg";
		case Sketcher::ConstraintType::DistanceX:
			return prefix + "Constraint_DistanceX.svg";
		case Sketcher::ConstraintType::DistanceY:
			return prefix + "Constraint_VerticalDistance.svg";
		case Sketcher::ConstraintType::Angle:
			return prefix + "Constraint_InternalAngle.svg";
		case Sketcher::ConstraintType::Perpendicular:
			return prefix + "Constraint_Perpendicular.svg";
		case Sketcher::ConstraintType::Radius:
			return prefix + "Constraint_Radius.svg";
		case Sketcher::ConstraintType::Equal:
			return prefix + "Constraint_EqualLength.svg";
		case Sketcher::ConstraintType::PointOnObject:
			return prefix + "Constraint_PointOnObject.svg";
		case Sketcher::ConstraintType::Symmetric:
			return prefix + "Constraint_Symmetric.svg";
		case Sketcher::ConstraintType::Diameter:
			return prefix + "Constraint_Diameter.svg";
		case Sketcher::ConstraintType::Block:
			return prefix + "Constraint_Block.svg";
		default:
			return QString();
		}
	}
	static QString curveIconPath(const Part::Geometry* g)
	{
		if (!g) {
			return QString();
		}
		const QString prefix = ":/widgets/icons/";
		if (g->is<Part::GeomLineSegment>()) {
			return prefix + "Sketcher_CreateLine.svg";
		}
		if (g->is<Part::GeomArcOfCircle>()) {
			return prefix + "Sketcher_CreateArc.svg";
		}
		if (g->is<Part::GeomCircle>()) {
			return prefix + "Sketcher_CreateCircle.svg";
		}
		if (g->is<Part::GeomEllipse>() || g->is<Part::GeomArcOfEllipse>()) {
			return prefix + "Sketcher_CreateEllipseByCenter.svg";
		}
		if (g->is<Part::GeomBSplineCurve>()) {
			return prefix + "Sketcher_CreateBSpline.svg";
		}
		if (g->is<Part::GeomPoint>()) {
			return prefix + "Sketcher_CreatePoint.svg";
		}
		return QString();
	}

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
        addParam(
            new BoolProperty(
                "Draw Grid",
                sketchGroup,
                BoolProperty::Style::InviwoRect
            )
        );
        addParam(
            new BoolProperty(
                "Snap To Grid",
                sketchGroup,
                BoolProperty::Style::InviwoRect
            )
        );
        addGroupParam("Constraints");
        addGroupParam("Curves");
        buildUi();

        mConstraintList = new QListWidget(this);
        mCurveList = new QListWidget(this);
        mConstraintList->setMinimumHeight(110);
        mCurveList->setMinimumHeight(110);
        auto attachList = [this](const QString& groupName, QListWidget* list) {
            const auto it = groupToIndex.find(groupName);
            if (it != groupToIndex.end()) {
                auto* group = m_comps[it->second].first;
                group->setCollapsed(false);
                group->addSubWidget(list);
            }
        };
        attachList("Constraints", mConstraintList);
        attachList("Curves", mCurveList);

        mRefreshTimer = new QTimer(this);
        mRefreshTimer->setInterval(300);
        connect(mRefreshTimer, &QTimer::timeout, this, [this]() { refreshLists(); });
        mRefreshTimer->start();
        refreshLists();
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
        if (propertyName == "Sketch:Snap To Grid" && mInternal->feature) {
            return QVariant::fromValue(mInternal->feature->getSketcherObj()->isSnapToGrid());
        }
        return QVariant();
    }

    void SketchTaskDialog::setParamValue(const QString& propertyName, const QVariant& value)
    {
        if (propertyName == "Sketch:Draw Grid" && mInternal->feature) {
            mInternal->feature->getSketcherObj()->setDrawGrid(value.value<bool>());
        }
        if (propertyName == "Sketch:Snap To Grid" && mInternal->feature) {
            mInternal->feature->getSketcherObj()->setSnapToGrid(value.value<bool>());
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
    void SketchTaskDialog::refreshLists()
    {
        if (!mInternal || !mInternal->feature || !mConstraintList || !mCurveList) {
            return;
        }
        SketcherObj* obj = mInternal->feature->getSketcherObj();
        if (!obj) {
            return;
        }

        QStringList constraintLines;
        QStringList constraintIconPaths;
        for (int i = 0; i < obj->getConstraintCount(); ++i) {
            const Sketcher::Constraint* c = obj->getConstraint(i);
            if (!c) {
                continue;
            }
            constraintIconPaths << constraintIconPath(c->Type);
            QString line = QString("%1  %2").arg(i).arg(c->typeToString().c_str());
            if (c->First != Sketcher::GeoEnum::GeoUndef) {
                line += QString("  F%1/%2").arg(c->First).arg(static_cast<int>(c->FirstPos));
            }
            if (c->Second != Sketcher::GeoEnum::GeoUndef) {
                line += QString("  S%1/%2").arg(c->Second).arg(static_cast<int>(c->SecondPos));
            }
            if (c->Third != Sketcher::GeoEnum::GeoUndef) {
                line += QString("  T%1/%2").arg(c->Third).arg(static_cast<int>(c->ThirdPos));
            }
            if (c->isDimensional()) {
                char buf[48] = { 0 };
                std::snprintf(buf, sizeof(buf), " = %.2f", c->getValue());
                line += buf;
            }
            constraintLines << line;
        }

        QStringList curveLines;
        QStringList curveIconPaths;
        for (int i = 0; i <= obj->getHighestCurveIndex(); ++i) {
            const Part::Geometry* g = obj->getGeometry(i);
            if (!g) {
                continue;
            }
            curveIconPaths << curveIconPath(g);
            QString typeName;
            if (g->is<Part::GeomLineSegment>()) {
                typeName = "Line";
            }
            else if (g->is<Part::GeomArcOfCircle>()) {
                typeName = "Arc";
            }
            else if (g->is<Part::GeomCircle>()) {
                typeName = "Circle";
            }
            else if (g->is<Part::GeomEllipse>()) {
                typeName = "Ellipse";
            }
            else if (g->is<Part::GeomBSplineCurve>()) {
                typeName = "BSpline";
            }
            else if (g->is<Part::GeomPoint>()) {
                typeName = "Point";
            }
            else {
                typeName = "Curve";
            }
            curveLines << QString("%1  %2").arg(i).arg(typeName);
        }

        const QString cache = constraintLines.join(QStringLiteral("\n")) + QStringLiteral("|")
            + curveLines.join(QStringLiteral("\n"));
        if (cache == mListCache) {
            return;
        }
        mListCache = cache;
        mConstraintList->clear();
        for (int i = 0; i < constraintLines.size(); ++i) {
            auto* item = new QListWidgetItem(
                constraintIconPaths[i].isEmpty()
                    ? QIcon()
                    : QIcon(constraintIconPaths[i]),
                constraintLines[i]
            );
            mConstraintList->addItem(item);
        }
        mCurveList->clear();
        for (int i = 0; i < curveLines.size(); ++i) {
            auto* item = new QListWidgetItem(
                curveIconPaths[i].isEmpty() ? QIcon() : QIcon(curveIconPaths[i]),
                curveLines[i]
            );
            mCurveList->addItem(item);
        }
    }
}
