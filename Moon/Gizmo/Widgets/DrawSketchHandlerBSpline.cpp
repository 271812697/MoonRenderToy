#pragma once
#include "Gizmo/Widgets/DrawSketchHandlerBSpline.h"
#include "Gizmo/Gizmo.h"
#include "base/Exception.h"
namespace MOON
{
	DrawSketchHandlerBSpline::DrawSketchHandlerBSpline(const std::string& name,BSplineConstructionMethod constrMethod,bool
        p):DrawSketchDefaultHandler<DrawSketchHandlerBSpline, StateMachines::ThreeSeekEnd, 2, BSplineConstructionMethod>(name), SplineDegree(3)
        , periodic(p)
        , prevCursorPosition(Base::Vector2d())
        , resetSeekSecond(false)
	{
		setActive(true);
	}
	DrawSketchHandlerBSpline::~DrawSketchHandlerBSpline()
	{
	}
	void DrawSketchHandlerBSpline::onUpdate()
	{
		renderer->pushSize(3);
		renderer->drawLine2D({ 100,0 }, { -100,0 });
		renderer->drawLine2D({ 0,100 }, { 0,-100 });
		for (int i = 0; i < lines.size(); i += 2) {
			renderer->drawLine2D({ lines[i].x
				,lines[i].y }, { lines[i + 1].x
				,lines[i + 1].y }, static_cast<MOON::Plane2D>(plane));
		}
		//renderer->drawCircle2D(m_internal->centerPoint,m_internal->radius);
		renderer->popSize();
	}
	void DrawSketchHandlerBSpline::onSetActive(bool flag)
	{
	}
	void DrawSketchHandlerBSpline::updateDataAndDrawToPosition(Base::Vector2d onSketchPos)
	{
		prevCursorPosition = onSketchPos;

		switch (state()) {
		case SelectMode::SeekFirst: {
			//toolWidgetManager.drawPositionAtCursor(onSketchPos);

			//seekAndRenderAutoConstraint(sugConstraints[0], onSketchPos, Base::Vector2d(0.f, 0.f));
		} break;
		case SelectMode::SeekSecond: {
			//toolWidgetManager.drawDirectionAtCursor(onSketchPos, getLastPoint());

			try {
				CreateAndDrawShapeGeometry();
			}
			catch (const Base::ValueError&) {
			}  // equal points while hovering raise an objection that can be safely ignored

			//seekAndRenderAutoConstraint(sugConstraints[1], onSketchPos, Base::Vector2d(0.f, 0.f));
		} break;
		default:
			break;
		}
	}
	bool DrawSketchHandlerBSpline::canGoToNextMode()
	{
        //Sketcher::PointPos pointPos = constructionMethod() == ConstructionMethod::ControlPoints
        //    ? Sketcher::PointPos::mid
        //    : Sketcher::PointPos::start;
        if (state() == SelectMode::SeekFirst) {
            // insert point for pole/knot, defer internal alignment constraining.
            if (!addPos()) {
                return false;
            }

            // add auto constraints on pole/knot
            //auto& ac0 = sugConstraints[0];
            //generateAutoConstraintsOnElement(ac0, geoIds.back(), pointPos);

            //sketchgui->getSketchObject()->solve();
        }
        else if (state() == SelectMode::SeekSecond) {
            // Prevent adding a new point if it's coincident with the last one.
            if (!points.empty()
                && (prevCursorPosition - getLastPoint()).Length() < Precision::Confusion()) {
                return false;
            }

            // We stay in SeekSecond unless the user closed the bspline.
            bool isClosed = false;

            // check if coincident with first pole/knot
            //for (auto& ac : sugConstraints.back()) {
            //    if (ac.Type == Sketcher::Coincident) {
            //        if (ac.GeoId == geoIds[0]) {
            //            isClosed = true;
            //        }
            //        else {
            //            // The coincidence with first point may be indirect
            //            const auto coincidents
            //                = sketchgui->getSketchObject()->getAllCoincidentPoints(ac.GeoId, ac.PosId);
            //            if (coincidents.find(geoIds[0]) != coincidents.end()) {
            //                isClosed = true;
            //            }
            //        }
            //    }
            //}

            if (isClosed) {
                if (periodic) {  // if periodic we do not need the last pole/knot
                    return true;
                }
            }
            else {
                //setAngleSnapping(true, getLastPoint());
                resetSeekSecond = true;
            }

            // insert circle point for pole/knot, defer internal alignment constraining.
            if (!addPos()) {
                return false;
            }

            // add auto constraints on pole/knot
 /*           auto& ac1 = sugConstraints[1];
            generateAutoConstraintsOnElement(ac1, geoIds.back(), pointPos);
            sugConstraintsBackup.push_back(std::move(ac1));
            ac1.clear();*/

            return isClosed;
        }
        return true;
	}
	void DrawSketchHandlerBSpline::quit()
	{
	}
	void DrawSketchHandlerBSpline::createShape(bool onlyeditoutline)
	{
        ShapeGeometry.clear();

        std::vector<Base::Vector3d> bsplinePoints3D;
        for (auto& point : points) {
            bsplinePoints3D.emplace_back(point.x, point.y, 0.0);
        }

        double len = (prevCursorPosition - getLastPoint()).Length();
        if (onlyeditoutline && (points.empty() || len >= Precision::Confusion())) {
            bsplinePoints3D.emplace_back(prevCursorPosition.x, prevCursorPosition.y, 0.0);
        }
        if (bsplinePoints3D.size() < 2) {
            return;
        }

        if (constructionMethod() == ConstructionMethod::ControlPoints) {
            size_t vSize = bsplinePoints3D.size();
            size_t maxDegree = vSize - (periodic ? 0 : 1);
            size_t degree = std::min(maxDegree, SplineDegree);

            std::vector<double> weights(vSize, 1.0);
            std::vector<double> knots;
            std::vector<int> mults;
            if (!periodic) {
                for (size_t i = 0; i < vSize - degree + 1; ++i) {
                    knots.push_back(i);
                }
                mults.resize(vSize - degree + 1, 1);
                mults.front() = degree + 1;
                mults.back() = degree + 1;
            }
            else {
                for (size_t i = 0; i < vSize + 1; ++i) {
                    knots.push_back(i);
                }
                mults.resize(vSize + 1, 1);
            }

            auto bSpline = std::make_unique<Part::GeomBSplineCurve>(
                bsplinePoints3D,
                weights,
                knots,
                mults,
                degree,
                periodic
            );
            bSpline->setPoles(bsplinePoints3D);
            //Sketcher::GeometryFacade::setConstruction(bSpline.get(), isConstructionMode());
            ShapeGeometry.emplace_back(std::move(bSpline));
        }
        else {
            try {
                std::vector<gp_Pnt> editCurveForOCCT;
                editCurveForOCCT.reserve(bsplinePoints3D.size());
                for (auto& p : bsplinePoints3D) {
                    editCurveForOCCT.emplace_back(p.x, p.y, 0.0);
                }

                // TODO: This maybe optimized by storing the spline as an attribute.
                auto bSpline = std::make_unique<Part::GeomBSplineCurve>();
                bSpline.get()->interpolate(editCurveForOCCT, periodic);

                //Sketcher::GeometryFacade::setConstruction(bSpline.get(), isConstructionMode());
                ShapeGeometry.emplace_back(std::move(bSpline));
            }
            catch (const Standard_Failure&) {
                // Since it happens very frequently that the interpolation fails
                // it's sufficient to report this as log message to avoid to pollute
                // the output window
               // Base::Console().log(std::string("drawBSplineToPosition"), "interpolation failed\n");
            }
        }
	}
    bool DrawSketchHandlerBSpline::addPos()
    {
        addToVectors();
        return addGeometry(getLastPoint(), geoIds.back(), points.size() == 1);
    }
    void DrawSketchHandlerBSpline::addToVectors()
    {
        points.push_back(prevCursorPosition);
        multiplicities.push_back(1);
        geoIds.push_back(1);
        //geoIds.push_back(getHighestCurveIndex() + 1);
        //if (geoIds.size() != distances.size()) {
        //    distances.push_back(-1);
        //}
    }
    bool DrawSketchHandlerBSpline::addGeometry(Base::Vector2d pos, int geoId, bool firstPoint)
    {
        try {
            //Gui::cmdAppObjectArgs(
            //    sketchgui->getObject(),
            //    constructionMethod() == ConstructionMethod::ControlPoints
            //    ? "addGeometry(Part.Circle(App.Vector(%f,%f,0),App.Vector(0,0,1),10),True)"
            //    : "addGeometry(Part.Point(App.Vector(%f,%f,0)),True)",
            //    pos.x,
            //    pos.y
            //);


            //if (constructionMethod() == ConstructionMethod::ControlPoints) {
            //    if (firstPoint) {  // First pole defaults to 1.0 weight
            //        Gui::cmdAppObjectArgs(
            //            sketchgui->getObject(),
            //            "addConstraint(Sketcher.Constraint('Weight',%d,%f)) ",
            //            geoId,
            //            1.0
            //        );
            //    }
            //    else {
            //        Gui::cmdAppObjectArgs(
            //            sketchgui->getObject(),
            //            "addConstraint(Sketcher.Constraint('Equal',%d,%d)) ",
            //            geoIds[0],
            //            geoId
            //        );
            //    }
            //}
        }
        catch (const Base::Exception&) {
            //Gui::NotifyError(
            //    sketchgui,
            //    QT_TRANSLATE_NOOP("Notifications", "Error"),
            //    QT_TRANSLATE_NOOP("Notifications", "Error adding B-spline pole/knot")
            //);

            //abortCommand();

            //sketchgui->getSketchObject()->solve();

            return false;
        }
        return true;
    }
    Base::Vector2d DrawSketchHandlerBSpline::getLastPoint()
    {
        return points.empty() ? Base::Vector2d() : points.back();
    }
}