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
		
	}
	DrawSketchHandlerBSpline::~DrawSketchHandlerBSpline()
	{
	}
	void DrawSketchHandlerBSpline::onUpdate()
	{
        DrawSketchHandler::onUpdate();
        Eigen::Vector4<uint8_t> normalColor = { 255,255,255,0 };
        Eigen::Vector4<uint8_t> hotColor = { 255,0,255,255 };
        Eigen::Vector4<uint8_t> activeColor = { 255,0,0,255 };
        Eigen::Vector4<uint8_t> tangentColor = { 255,255,255,255 };
        renderer->pushColor({255,255,255,0});
        for (int i = 0;i < points.size();i++) {
            renderer->drawPoint2D({ points[i].x,points[i].y }, i == hotPointId?(isPointActive? activeColor:hotColor) : normalColor, 15, gizmoPlane);
        }
        if (constructionMethod() == ConstructionMethod::Knots) {
            for (int i = 0;i < tangents.size();i++) {
			    Base::Vector2d tangentE = points[i] + tangents[i];
                Base::Vector2d tangentS = points[i] - tangents[i];
			    renderer->drawPoint2D({ tangentE.x,tangentE.y }, i == hotTangentId ? (isTangentActive ? activeColor : hotColor) : tangentColor, 15,gizmoPlane);
                renderer->drawPoint2D({ tangentS.x,tangentS.y }, i == hotTangentId ? (isTangentActive ? activeColor : hotColor) : tangentColor, 15, gizmoPlane);
                renderer->drawLine2D({ tangentS.x,tangentS.y }, { tangentE.x,tangentE.y }, gizmoPlane);
            }
        }
        renderer->popColor();
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

        if (state() == SelectMode::SeekSecond) {
            if (geoIds.size() > 1) {
                // create B-spline from existing poles/knots
                setState(SelectMode::End);
                finish();
            }
            else {
                // We don't want to finish() as that'll create auto-constraints
                handleContinuousMode();
            }
        }
        else {
            DrawSketchHandler::quit();
        }
	}
    void DrawSketchHandlerBSpline::rightButtonOrEsc()
    {
        quit();
    }
	void DrawSketchHandlerBSpline::createShape(bool onlyeditoutline)
	{
        ShapeGeometry.clear();

        std::vector<Base::Vector3d> bsplinePoints3D;
        for (auto& point : points) {
            bsplinePoints3D.emplace_back(point.x, point.y, 0.0);
        }

        //double len = (prevCursorPosition - getLastPoint()).Length();
        //if (onlyeditoutline && (points.empty() || len >= Precision::Confusion())) {
        //    bsplinePoints3D.emplace_back(prevCursorPosition.x, prevCursorPosition.y, 0.0);
        //}
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
				std::vector<gp_Vec> editTangentsForOCCT;
                editTangentsForOCCT.reserve(tangents.size());
                for (auto& t : tangents) {
					editTangentsForOCCT.emplace_back(t.x, t.y, 0.0);
                }
                // TODO: This maybe optimized by storing the spline as an attribute.
                auto bSpline = std::make_unique<Part::GeomBSplineCurve>();
                bSpline.get()->interpolate(editCurveForOCCT, editTangentsForOCCT,periodic);

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
        if (hotPointId != -1|| hotTangentId!=-1) {
            return;
        }
        if (points.size() > 0) {
			Base::Vector2d delta = prevCursorPosition - points.back();
            tangents.push_back(delta.Normalize()*10);
        }
        else
        {
            tangents.push_back(Base::Vector2d(1.0,0.0));
        }
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
    void DrawSketchHandlerBSpline::onLeftMousePressed()
    {
        SuperClass::onLeftMousePressed();
        if (hotPointId!=-1) {
            isPointActive = true;
        }
        else
        {
            isPointActive = false;
            if (constructionMethod() == ConstructionMethod::Knots) {
                if (hotTangentId != -1) {
				    isTangentActive = true;
			    }
                else
                {
                    isTangentActive = false;
                }
            }
        }
    }
    void DrawSketchHandlerBSpline::onLeftMouseReleased()
    {
        SuperClass::onRightMouseReleased();
        isPointActive = false;
        isTangentActive = false;
    }
    void DrawSketchHandlerBSpline::onMouseMove()
	{
		SuperClass::onMouseMove();
        if (isPointActive) {
            if(hotPointId!=-1)
			points[hotPointId] = onSketchPos;
		}
		else if (isTangentActive && constructionMethod() == ConstructionMethod::Knots) {
            if (hotTangentId != -1) {
                if (isForwardTangent) {
					tangents[hotTangentId] = onSketchPos - points[hotTangentId];
                }
                else
                {
					tangents[hotTangentId] = points[hotTangentId] - onSketchPos;
                }
            }
        }
        else
        {
            hotPointId = -1;
            hotTangentId = -1;
            double tolerance = 2.0;
            for (int i = 0;i < points.size();i++) {
                if ((onSketchPos - points[i]).Length() < tolerance) {
                    hotPointId = i;
                }
            }
            if (hotPointId == -1&& constructionMethod() == ConstructionMethod::Knots) {
                for (int i = 0;i < tangents.size();i++) {
                    Base::Vector2d pos = points[i] + tangents[i];
                    if ((onSketchPos - pos).Length() < tolerance) {
                        hotTangentId = i;
                        isForwardTangent = true;
                    }
                    pos = points[i] - tangents[i];
                    if ((onSketchPos - pos).Length() < tolerance) {
                        hotTangentId = i;
                        isForwardTangent = false;
                    }
                }
            }
        }
    }
    void DrawSketchHandlerBSpline::onReset()
	{
		SuperClass::onReset();
		points.clear();
		tangents.clear();
		multiplicities.clear();
		geoIds.clear();
        SplineDegree = 3;
		isPointActive = false;
		isTangentActive = false;
		hotPointId = -1;
		hotTangentId = -1;
    }
    void DrawSketchHandlerBSpline::onKeyPress(const std::string& key)
    {
        SuperClass::onKeyPress(key);
        if (key=="P") {
            periodic = !periodic;
        }
        else if (key=="B") {
            if (points.size()) {
                points.pop_back();
                tangents.pop_back();
                multiplicities.pop_back();
                geoIds.pop_back();
            }
        }
    }
}
