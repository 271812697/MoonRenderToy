#include "Interactive/Widgets/DrawSketchHandlerLineSet.h"
#include "Sketcher/SketcherObjManager.h"
#include "Sketcher/SketcherObj.h"
#include "Maths/FMatrix4.h"
#include "renderer/SceneView.h"
#include <numbers>
#include <boost/math/special_functions/fpclassify.hpp>


namespace MOON {
	DrawSketchHandlerLineSet::DrawSketchHandlerLineSet(const std::string& name):
		DrawSketchHandler(name),
		Mode(STATUS_SEEK_First)
		, SegmentMode(SEGMENT_MODE_Line)
		, TransitionMode(TRANSITION_MODE_Free)
		, SnapMode(SNAP_MODE_Free)
		, suppressTransition(false)
		, EditCurve(2)
		, firstCurve(-1)
		, previousCurve(-1)
		, firstPosId(SketcherObj::PointPos::none)
		, previousPosId(SketcherObj::PointPos::none)
		, startAngle(0)
		, endAngle(0)
		, arcRadius(0)
		, firstsegment(true)
	{
		
		setActive(false);
		makePlane(SketcherPlane2D());
	}
	DrawSketchHandlerLineSet::~DrawSketchHandlerLineSet()
	{
	}
	void DrawSketchHandlerLineSet::onUpdate()
	{
		DrawSketchHandler::onUpdate();
	}
	void DrawSketchHandlerLineSet::onLeftMousePressed()
	{
        onButtonPressed(onSketchPos);
	}
	void DrawSketchHandlerLineSet::onLeftMouseReleased()
	{
        onButtonReleased(onSketchPos);
	}
	void DrawSketchHandlerLineSet::onRightMousePressed()
	{
        quit();
	}
	void DrawSketchHandlerLineSet::onRightMouseReleased()
	{
	}
	void DrawSketchHandlerLineSet::onMouseMove()
	{
		DrawSketchHandler::onMouseMove();
        mouseMove(onSketchPos);
	}
	void DrawSketchHandlerLineSet::onKeyPress(const std::string& key)
	{
        if (key == "CONTROL_L") {
            ctrlDown = true;
        }
        SketcherObj* obj = SketcherObjManager::instance().GetCurrentActiveSketcherObj();
        if (Mode == STATUS_SEEK_Second && key == "M"
            && previousCurve != -1) {
            // loop through the following modes:
            // SEGMENT_MODE_Line, TRANSITION_MODE_Free / TRANSITION_MODE_Tangent
            // SEGMENT_MODE_Line, TRANSITION_MODE_Perpendicular_L
            // SEGMENT_MODE_Line, TRANSITION_MODE_Tangent / TRANSITION_MODE_Free
            // SEGMENT_MODE_Arc, TRANSITION_MODE_Tangent
            // SEGMENT_MODE_Arc, TRANSITION_MODE_Perpendicular_L
            // SEGMENT_MODE_Arc, TRANSITION_MODE_Perpendicular_R

            SnapMode = SNAP_MODE_Free;

            Base::Vector2d onSketchPos;
            if (SegmentMode == SEGMENT_MODE_Line) {
                onSketchPos = EditCurve[EditCurve.size() - 1];
            }
            else {
                onSketchPos = EditCurve[29];
            }

            const Part::Geometry* geom = obj->getGeometry(previousCurve);

            if (SegmentMode == SEGMENT_MODE_Line) {
                switch (TransitionMode) {
                case TRANSITION_MODE_Free:
                    if (geom->is<Part::GeomArcOfCircle>()) {  // 3rd mode
                        SegmentMode = SEGMENT_MODE_Arc;
                        TransitionMode = TRANSITION_MODE_Tangent;
                    }
                    else {  // 1st mode
                        TransitionMode = TRANSITION_MODE_Perpendicular_L;
                    }
                    break;
                case TRANSITION_MODE_Perpendicular_L:  // 2nd mode
                    if (geom->is<Part::GeomArcOfCircle>()) {
                        TransitionMode = TRANSITION_MODE_Free;
                    }
                    else {
                        TransitionMode = TRANSITION_MODE_Tangent;
                    }
                    break;
                case TRANSITION_MODE_Tangent:
                    if (geom->is<Part::GeomArcOfCircle>()) {  // 1st mode
                        TransitionMode = TRANSITION_MODE_Perpendicular_L;
                    }
                    else {  // 3rd mode
                        SegmentMode = SEGMENT_MODE_Arc;
                        TransitionMode = TRANSITION_MODE_Tangent;
                    }
                    break;
                default:  // unexpected mode
                    TransitionMode = TRANSITION_MODE_Free;
                    break;
                }
            }
            else {
                switch (TransitionMode) {
                case TRANSITION_MODE_Tangent:  // 4th mode
                    TransitionMode = TRANSITION_MODE_Perpendicular_L;
                    break;
                case TRANSITION_MODE_Perpendicular_L:  // 5th mode
                    TransitionMode = TRANSITION_MODE_Perpendicular_R;
                    break;
                default:  // 6th mode (Perpendicular_R) + unexpected mode
                    SegmentMode = SEGMENT_MODE_Line;
                    if (geom->is<Part::GeomArcOfCircle>()) {
                        TransitionMode = TRANSITION_MODE_Tangent;
                    }
                    else {
                        TransitionMode = TRANSITION_MODE_Free;
                    }
                    break;
                }
            }

            if (SegmentMode == SEGMENT_MODE_Line) {
                EditCurve.resize(TransitionMode == TRANSITION_MODE_Free ? 2 : 3);
            }
            else {
                EditCurve.resize(32);
            }

            mouseMove(onSketchPos);  // trigger an update of EditCurve
        }
        else {
            //DrawSketchHandler::registerPressedKey(pressed, key);
        }
	}
    void DrawSketchHandlerLineSet::onButtonPressed(Base::Vector2d pos)
    {
        SketcherObj* obj = SketcherObjManager::instance().GetCurrentActiveSketcherObj();
       
        if (Mode == STATUS_SEEK_First) {

            EditCurve[0] = onSketchPos;  // this may be overwritten if previousCurve is found
            SketcherObj::SelectGeoId preSelectId= obj->testSelect(onSketchPos);
            if (preSelectId.GeoId != -1&&(preSelectId.pointPos== SketcherObj::PointPos::start|| preSelectId.pointPos == SketcherObj::PointPos::end)) {
                Part::Geometry* geo= obj->getGeometry(preSelectId.GeoId);
                if (geo->is<Part::GeomLineSegment>() || geo->is<Part::GeomArcOfCircle>()) {
                    previousCurve = preSelectId.GeoId;
                    previousPosId = preSelectId.pointPos;
                    updateTransitionData(previousCurve,
                        previousPosId);
                    if (geo->is<Part::GeomArcOfCircle>()) {
                        TransitionMode = TRANSITION_MODE_Tangent;
                        SnapMode = SNAP_MODE_Free;
                    }
                }
            }


            //virtualsugConstr1 = sugConstr1;  // store original autoconstraints.

            // here we check if there is a preselected point and
            // we set up a transition from the neighbouring segment.
            // (peviousCurve, previousPosId, dirVec, TransitionMode)
            

            // remember our first point (even if we are doing a transition from a previous curve)
            firstCurve = obj->getHighestCurveIndex() + 1;
            firstPosId = SketcherObj::PointPos::start;

            if (SegmentMode == SEGMENT_MODE_Line) {
                EditCurve.resize(TransitionMode == TRANSITION_MODE_Free ? 2 : 3);
            }
            else if (SegmentMode == SEGMENT_MODE_Arc) {
                EditCurve.resize(32);
            }
            Mode = STATUS_SEEK_Second;
        }
        else if (Mode == STATUS_SEEK_Second) {
            // exit on clicking exactly at the same position (e.g. double click)
            if (onSketchPos == EditCurve[0]) {
                //unsetCursor();
                //resetPositionText();
                EditCurve.clear();
                clearEdit();
                drawEdit(EditCurve);

               
                bool continuousMode = true;// hGrp->GetBool("ContinuousCreationMode", true);

                if (continuousMode) {
                    // This code enables the continuous creation mode.
                    Mode = STATUS_SEEK_First;
                    SegmentMode = SEGMENT_MODE_Line;
                    TransitionMode = TRANSITION_MODE_Free;
                    SnapMode = SNAP_MODE_Free;
                    suppressTransition = false;
                    firstCurve = -1;
                    previousCurve = -1;
                    firstPosId = SketcherObj::PointPos::none;
                    previousPosId = SketcherObj::PointPos::none;
                    EditCurve.clear();
                    clearEdit();
                    drawEdit(EditCurve);
                    EditCurve.resize(2);
                    //applyCursor();
                    /* this is ok not to call to purgeHandler
                     * in continuous creation mode because the
                     * handler is destroyed by the quit() method on pressing the
                     * right button of the mouse */
                    return ;
                }
                else {
                    //sketchgui->purgeHandler();  // no code after this line, Handler get deleted in
                    // ViewProvider
                    return ;
                }
            }

            Mode = STATUS_Do;
            auto preSelect=obj->getPreSelectGeoId();
            if (preSelect.GeoId!= -1 && firstPosId != SketcherObj::PointPos::none) {
                if (firstCurve == preSelect.GeoId && firstPosId == firstPosId) {
                    Mode = STATUS_Close;
                }
                //int GeoId;
                //Sketcher::PointPos PosId;
                //sketchgui->getSketchObject()->getGeoVertexIndex(getPreselectPoint(), GeoId, PosId);
                //if (sketchgui->getSketchObject()
                //    ->arePointsCoincident(GeoId, PosId, firstCurve, firstPosId)) {
                //    Mode = STATUS_Close;
                //}
            }
            //else if (getPreselectCross() == 0 && firstPosId != Sketcher::PointPos::none) {
            //    // close line started at root point
            //    if (sketchgui->getSketchObject()
            //        ->arePointsCoincident(-1, Sketcher::PointPos::start, firstCurve, firstPosId)) {
            //        Mode = STATUS_Close;
            //    }
            //}
        }

        //updateHint();

        
    }
    void DrawSketchHandlerLineSet::onButtonReleased(Base::Vector2d pos)
    {
        SketcherObj* obj = SketcherObjManager::instance().GetCurrentActiveSketcherObj();
        if (Mode == STATUS_Do || Mode == STATUS_Close) {
            bool addedGeometry = true;
            if (SegmentMode == SEGMENT_MODE_Line) {
                // issue the geometry
                try {
                    // open the transaction
                    //openCommand(QT_TRANSLATE_NOOP("Command", "Add line to sketch polyline"));
                    auto line = std::make_unique<Part::GeomLineSegment>();
                    line->setPoints(Base::Vector3d(EditCurve[0].x,
                        EditCurve[0].y,0), Base::Vector3d(EditCurve[1].x,
                            EditCurve[1].y,0));
                    obj->addGeometry(line.get());
                }
                catch (const Base::Exception&) {
                    addedGeometry = false;
                   /* Gui::NotifyError(
                        sketchgui,
                        QT_TRANSLATE_NOOP("Notifications", "Error"),
                        QT_TRANSLATE_NOOP("Notifications", "Failed to add line")
                    );
                    abortCommand();*/
                }

                firstsegment = false;
            }
            else if (SegmentMode == SEGMENT_MODE_Arc) {  // We're dealing with an Arc
                if (!boost::math::isnormal(arcRadius)) {
                    Mode = STATUS_SEEK_Second;
                    return ;
                }

                try {
                    auto arc = std::make_unique<Part::GeomArcOfCircle>();
                    arc->setCenter(Base::Vector3d(CenterPoint.x,
                        CenterPoint.y, 0));
                    arc->setRange(std::min(startAngle, endAngle), std::max(startAngle, endAngle), true);
                    arc->setRadius(std::abs(arcRadius));
                    obj->addGeometry(arc.get());
                }
                catch (const Base::Exception&) {
                    addedGeometry = false;
   /*                 Gui::NotifyError(
                        sketchgui,
                        QT_TRANSLATE_NOOP("Notifications", "Error"),
                        QT_TRANSLATE_NOOP("Notifications", "Failed to add arc")
                    );

                    abortCommand();*/
                }

                firstsegment = false;
            }

            int lastCurve = obj->getHighestCurveIndex();
            // issue the constraint
            //if (addedGeometry && (previousPosId != Sketcher::PointPos::none)) {
            //    Sketcher::PointPos lastStartPosId = (SegmentMode == SEGMENT_MODE_Arc
            //        && startAngle > endAngle)
            //        ? Sketcher::PointPos::end
            //        : Sketcher::PointPos::start;
            //    Sketcher::PointPos lastEndPosId = (SegmentMode == SEGMENT_MODE_Arc
            //        && startAngle > endAngle)
            //        ? Sketcher::PointPos::start
            //        : Sketcher::PointPos::end;
            //    // in case of a tangency constraint, the coincident constraint is redundant
            //    std::string constrType = "Coincident";
            //    if (!suppressTransition && previousCurve != -1) {
            //        if (TransitionMode == TRANSITION_MODE_Tangent) {
            //            constrType = "Tangent";
            //        }
            //        else if (
            //            TransitionMode == TRANSITION_MODE_Perpendicular_L
            //            || TransitionMode == TRANSITION_MODE_Perpendicular_R
            //            ) {
            //            constrType = "Perpendicular";
            //        }
            //    }
            //    Gui::cmdAppObjectArgs(
            //        sketchgui->getObject(),
            //        "addConstraint(Sketcher.Constraint('%s',%i,%i,%i,%i)) ",
            //        constrType.c_str(),
            //        previousCurve,
            //        static_cast<int>(previousPosId),
            //        lastCurve,
            //        static_cast<int>(lastStartPosId)
            //    );

            //    if (SnapMode == SNAP_MODE_45Degree && Mode != STATUS_Close) {
            //        // -360, -315, -270, -225, -180, -135, -90, -45,  0, 45,  90, 135, 180, 225,
            //        // 270, 315, 360
            //        //  N/A,    a, perp,    a,  par,    a,perp,   a,N/A,  a,perp,   a, par, a,perp,
            //        //  a, N/A

            //        // #3974: if in radians, the printf %f defaults to six decimals, which leads to
            //        // loss of precision
            //        double arcAngle = abs(
            //            round((endAngle - startAngle) / (std::numbers::pi / 4)) * 45
            //        );  // in degrees

            //        Gui::cmdAppObjectArgs(
            //            sketchgui->getObject(),
            //            "addConstraint(Sketcher.Constraint('Angle',%i,App.Units."
            //            "Quantity('%f deg'))) ",
            //            lastCurve,
            //            arcAngle
            //        );
            //    }
            //    if (Mode == STATUS_Close) {
            //        // close the loop by constrain to the first curve point
            //        Gui::cmdAppObjectArgs(
            //            sketchgui->getObject(),
            //            "addConstraint(Sketcher.Constraint('Coincident',%i,%i,%i,%i)) ",
            //            lastCurve,
            //            static_cast<int>(lastEndPosId),
            //            firstCurve,
            //            static_cast<int>(firstPosId)
            //        );
            //        firstsegment = true;
            //    }
            //    commitCommand();

            //    tryAutoRecomputeIfNotSolve(sketchgui->getObject<Sketcher::SketchObject>());
            //}

            //ParameterGrp::handle hGrp = App::GetApplication().GetParameterGroupByPath(
            //    "User parameter:BaseApp/Preferences/Mod/Sketcher"
            //);
            //bool avoidredundant = sketchgui->AvoidRedundant.getValue()
            //    && sketchgui->Autoconstraints.getValue();

            if (Mode == STATUS_Close) {

                //if (avoidredundant) {
                //    if (SegmentMode == SEGMENT_MODE_Line) {  // avoid redundant constraints.
                //        if (sugConstr1.size() > 0) {
                //            removeRedundantHorizontalVertical(
                //                sketchgui->getObject<Sketcher::SketchObject>(),
                //                sugConstr1,
                //                sugConstr2
                //            );
                //        }
                //        else {
                //            removeRedundantHorizontalVertical(
                //                sketchgui->getObject<Sketcher::SketchObject>(),
                //                virtualsugConstr1,
                //                sugConstr2
                //            );
                //        }
                //    }
                //}

                //if (!sugConstr2.empty()) {
                //    // exclude any coincidence constraints
                //    std::vector<AutoConstraint> sugConstr;
                //    for (unsigned int i = 0; i < sugConstr2.size(); i++) {
                //        if (sugConstr2[i].Type != Sketcher::Coincident) {
                //            sugConstr.push_back(sugConstr2[i]);
                //        }
                //    }
                //    createAutoConstraints(sugConstr, getHighestCurveIndex(), Sketcher::PointPos::end);
                //    sugConstr2.clear();
                //}

                //tryAutoRecomputeIfNotSolve(sketchgui->getObject<Sketcher::SketchObject>());

                //unsetCursor();

                //resetPositionText();
                EditCurve.clear();
                clearEdit();
                drawEdit(EditCurve);

                bool continuousMode = true;// hGrp->GetBool("ContinuousCreationMode", true);

                if (continuousMode) {
                    // This code enables the continuous creation mode.
                    Mode = STATUS_SEEK_First;
                    SegmentMode = SEGMENT_MODE_Line;
                    TransitionMode = TRANSITION_MODE_Free;
                    SnapMode = SNAP_MODE_Free;
                    suppressTransition = false;
                    firstCurve = -1;
                    previousCurve = -1;
                    firstPosId = SketcherObj::PointPos::none;
                    previousPosId = SketcherObj::PointPos::none;
                    EditCurve.clear();
                    clearEdit();
                    drawEdit(EditCurve);
                    EditCurve.resize(2);
                   // applyCursor();
                    /* this is ok not to call to purgeHandler
                     * in continuous creation mode because the
                     * handler is destroyed by the quit() method on pressing the
                     * right button of the mouse */
                }
                else {
                   // sketchgui->purgeHandler();  // no code after this line, Handler get deleted in
                    // ViewProvider
                }
            }
            else {
                //commitCommand();

                //// Add auto constraints
                //if (!sugConstr1.empty()) {  // this is relevant only to the very first point
                //    createAutoConstraints(sugConstr1, getHighestCurveIndex(), Sketcher::PointPos::start);
                //    sugConstr1.clear();
                //}


                //if (avoidredundant) {
                //    if (SegmentMode == SEGMENT_MODE_Line) {  // avoid redundant constraints.
                //        if (sugConstr1.size() > 0) {
                //            removeRedundantHorizontalVertical(
                //                sketchgui->getObject<Sketcher::SketchObject>(),
                //                sugConstr1,
                //                sugConstr2
                //            );
                //        }
                //        else {
                //            removeRedundantHorizontalVertical(
                //                sketchgui->getObject<Sketcher::SketchObject>(),
                //                virtualsugConstr1,
                //                sugConstr2
                //            );
                //        }
                //    }
                //}

                //virtualsugConstr1 = sugConstr2;  // these are the initial constraints for the next
                // iteration.

                //if (!sugConstr2.empty()) {
                //    createAutoConstraints(
                //        sugConstr2,
                //        getHighestCurveIndex(),
                //        (SegmentMode == SEGMENT_MODE_Arc && startAngle > endAngle)
                //        ? Sketcher::PointPos::start
                //        : Sketcher::PointPos::end
                //    );
                //    sugConstr2.clear();
                //}

                //tryAutoRecomputeIfNotSolve(sketchgui->getObject<Sketcher::SketchObject>());

                // remember the vertex for the next rounds constraint..
                previousCurve = obj->getHighestCurveIndex();
                previousPosId = (SegmentMode == SEGMENT_MODE_Arc && startAngle > endAngle)
                    ? SketcherObj::PointPos::start
                    : SketcherObj::PointPos::end;  // cw arcs are rendered in reverse

                // setup for the next line segment
                // calculate dirVec and EditCurve[0]
                updateTransitionData(previousCurve, previousPosId);

                //applyCursor();
                Mode = STATUS_SEEK_Second;
                if (SegmentMode == SEGMENT_MODE_Arc) {
                    TransitionMode = TRANSITION_MODE_Tangent;
                    EditCurve.resize(3);
                    EditCurve[2] = EditCurve[0];
                }
                else {
                    TransitionMode = TRANSITION_MODE_Free;
                    EditCurve.resize(2);
                }
                SegmentMode = SEGMENT_MODE_Line;
                SnapMode = SNAP_MODE_Free;
                EditCurve[1] = EditCurve[0];
            //if (obj) {
            //    Maths::FMatrix4 mat = m_sceneView->GetCamera()->GetViewPortMatrix();
            //    Base::Matrix4D pla(
            //        mat.data[0], mat.data[1], mat.data[2], mat.data[3],
            //        mat.data[4], mat.data[5], mat.data[6], mat.data[7],
            //        mat.data[8], mat.data[9], mat.data[10], mat.data[11],
            //        mat.data[12], mat.data[13], mat.data[14], mat.data[15]
            //    );
            //    isSnapedSketchPos = obj->snapPoint(onSketchPos, pla);
            //}
            mouseMove(onSketchPos);  // trigger an update of EditCurve
            }
        }

       // updateHint();
    }
    void DrawSketchHandlerLineSet::mouseMove(Base::Vector2d pos) {
        using std::numbers::pi;
        Base::Vector2d onSketchPos = pos;

        suppressTransition = false;
        if (Mode == STATUS_SEEK_First) {
            //setPositionText(onSketchPos);
            //seekAndRenderAutoConstraint(sugConstr1, onSketchPos, Base::Vector2d(0.f, 0.f));
        }
        else if (Mode == STATUS_SEEK_Second) {
            if (SegmentMode == SEGMENT_MODE_Line) {
                EditCurve[EditCurve.size() - 1] = onSketchPos;
                if (TransitionMode == TRANSITION_MODE_Tangent) {
                    Base::Vector2d Tangent(dirVec.x, dirVec.y);
                    EditCurve[1].ProjectToLine(EditCurve[2] - EditCurve[0], Tangent);
                    if (EditCurve[1] * Tangent < 0) {
                        EditCurve[1] = EditCurve[2];
                        suppressTransition = true;
                    }
                    else {
                        EditCurve[1] = EditCurve[0] + EditCurve[1];
                    }
                }
                else if (
                    TransitionMode == TRANSITION_MODE_Perpendicular_L
                    || TransitionMode == TRANSITION_MODE_Perpendicular_R
                    ) {
                    Base::Vector2d Perpendicular(-dirVec.y, dirVec.x);
                    EditCurve[1].ProjectToLine(EditCurve[2] - EditCurve[0], Perpendicular);
                    EditCurve[1] = EditCurve[0] + EditCurve[1];
                }
                clearEdit();
                drawEdit(EditCurve);

                float length = (EditCurve[1] - EditCurve[0]).Length();
                float angle = (EditCurve[1] - EditCurve[0]).GetAngle(Base::Vector2d(1.f, 0.f));

                //if (showCursorCoords()) {
                //    SbString text;
                //    std::string lengthString = lengthToDisplayFormat(length, 1);
                //    std::string angleString = angleToDisplayFormat(angle * 180.0 / pi, 1);
                //    text.sprintf(" (%s, %s)", lengthString.c_str(), angleString.c_str());
                //    setPositionText(EditCurve[1], text);
                //}

                //if (TransitionMode == TRANSITION_MODE_Free) {
                //    seekAndRenderAutoConstraint(sugConstr2, onSketchPos, onSketchPos - EditCurve[0]);
                //}
            }
            else if (SegmentMode == SEGMENT_MODE_Arc) {

                if (ctrlDown) {
                    SnapMode = SNAP_MODE_45Degree;
                }
                else {
                    SnapMode = SNAP_MODE_Free;
                }

                Base::Vector2d Tangent;
                if (TransitionMode == TRANSITION_MODE_Tangent) {
                    Tangent = Base::Vector2d(dirVec.x, dirVec.y);
                }
                else if (TransitionMode == TRANSITION_MODE_Perpendicular_L) {
                    Tangent = Base::Vector2d(-dirVec.y, dirVec.x);
                }
                else if (TransitionMode == TRANSITION_MODE_Perpendicular_R) {
                    Tangent = Base::Vector2d(dirVec.y, -dirVec.x);
                }

                double theta = Tangent.GetAngle(onSketchPos - EditCurve[0]);

                arcRadius = (onSketchPos - EditCurve[0]).Length() / (2.0 * sin(theta));

                // At this point we need a unit normal vector pointing towards
                // the center of the arc we are drawing. Derivation of the formula
                // used here can be found at
                // http://people.richland.edu/james/lecture/m116/matrices/area.html
                double x1 = EditCurve[0].x;
                double y1 = EditCurve[0].y;
                double x2 = x1 + Tangent.x;
                double y2 = y1 + Tangent.y;
                double x3 = onSketchPos.x;
                double y3 = onSketchPos.y;
                if ((x2 * y3 - x3 * y2) - (x1 * y3 - x3 * y1) + (x1 * y2 - x2 * y1) > 0) {
                    arcRadius *= -1;
                }
                if (boost::math::isnan(arcRadius) || boost::math::isinf(arcRadius)) {
                    arcRadius = 0.f;
                }

                CenterPoint = EditCurve[0]
                    + Base::Vector2d(arcRadius * Tangent.y, -arcRadius * Tangent.x);

                double rx = EditCurve[0].x - CenterPoint.x;
                double ry = EditCurve[0].y - CenterPoint.y;

                startAngle = atan2(ry, rx);

                double rxe = onSketchPos.x - CenterPoint.x;
                double rye = onSketchPos.y - CenterPoint.y;
                double arcAngle = atan2(-rxe * ry + rye * rx, rxe * rx + rye * ry);
                if (boost::math::isnan(arcAngle) || boost::math::isinf(arcAngle)) {
                    arcAngle = 0.f;
                }
                if (arcRadius >= 0 && arcAngle > 0) {
                    arcAngle -= 2 * pi;
                }
                if (arcRadius < 0 && arcAngle < 0) {
                    arcAngle += 2 * pi;
                }

                if (SnapMode == SNAP_MODE_45Degree) {
                    arcAngle = round(arcAngle / (pi / 4)) * pi / 4;
                }

                endAngle = startAngle + arcAngle;

                for (int i = 1; i <= 29; i++) {
                    double angle = i * arcAngle / 29.0;
                    double dx = rx * cos(angle) - ry * sin(angle);
                    double dy = rx * sin(angle) + ry * cos(angle);
                    EditCurve[i] = Base::Vector2d(CenterPoint.x + dx, CenterPoint.y + dy);
                }

                EditCurve[30] = CenterPoint;
                EditCurve[31] = EditCurve[0];
                clearEdit();
                drawEdit(EditCurve);

             /*   if (showCursorCoords()) {
                    SbString text;
                    std::string radiusString = lengthToDisplayFormat(std::abs(arcRadius), 1);
                    std::string angleString = angleToDisplayFormat(arcAngle * 180.0 / pi, 1);
                    text.sprintf(" (R%s, %s)", radiusString.c_str(), angleString.c_str());
                    setPositionText(onSketchPos, text);
                }
                seekAndRenderAutoConstraint(sugConstr2, onSketchPos, Base::Vector2d(0.f, 0.f));*/
            }
        }
    }
    void DrawSketchHandlerLineSet::quit()
    {
        // We must see if we need to create a B-spline before cancelling everything
// and now just like any other Handler,

      
        bool continuousMode = true;

        if (firstsegment) {
            // user when right-clicking with no segment in really wants to exit
            DrawSketchHandler::quit();
        }
        else {

            if (!continuousMode) {
                DrawSketchHandler::quit();
            }
            else {
                // This code disregards existing data and enables the continuous creation mode.
                Mode = STATUS_SEEK_First;
                SegmentMode = SEGMENT_MODE_Line;
                TransitionMode = TRANSITION_MODE_Free;
                SnapMode = SNAP_MODE_Free;
                suppressTransition = false;
                firstCurve = -1;
                previousCurve = -1;
                firstPosId = SketcherObj::PointPos::none;
                previousPosId = SketcherObj::PointPos::none;
                firstsegment = true;
                EditCurve.clear();
                clearEdit();
                drawEdit(EditCurve);
                EditCurve.resize(2);
               
            }
        }
    }
    void DrawSketchHandlerLineSet::updateTransitionData(int GeoId, SketcherObj::PointPos PosId)
    {
        SketcherObj* obj = SketcherObjManager::instance().GetCurrentActiveSketcherObj();
        // Use updated startPoint/endPoint as autoconstraints can modify the position
        const Part::Geometry* geom = obj->getGeometry(GeoId);
        if (geom->is<Part::GeomLineSegment>()) {
            const Part::GeomLineSegment* lineSeg = static_cast<const Part::GeomLineSegment*>(geom);
            dirVec.Set(
                lineSeg->getEndPoint().x - lineSeg->getStartPoint().x,
                lineSeg->getEndPoint().y - lineSeg->getStartPoint().y,
                0.f
            );
            if (PosId == SketcherObj::PointPos::start) {
                dirVec *= -1;
                EditCurve[0] = Base::Vector2d(lineSeg->getStartPoint().x, lineSeg->getStartPoint().y);
            }
            else {
                EditCurve[0] = Base::Vector2d(lineSeg->getEndPoint().x, lineSeg->getEndPoint().y);
            }
        }
        else if (geom->is<Part::GeomArcOfCircle>()) {
            const Part::GeomArcOfCircle* arcSeg = static_cast<const Part::GeomArcOfCircle*>(geom);
            if (PosId == SketcherObj::PointPos::start) {
                EditCurve[0] = Base::Vector2d(
                    arcSeg->getStartPoint(/*emulateCCW=*/true).x,
                    arcSeg->getStartPoint(/*emulateCCW=*/true).y
                );
                dirVec = Base::Vector3d(0.f, 0.f, -1.0)
                    % (arcSeg->getStartPoint(/*emulateCCW=*/true) - arcSeg->getCenter());
            }
            else {
                EditCurve[0] = Base::Vector2d(
                    arcSeg->getEndPoint(/*emulateCCW=*/true).x,
                    arcSeg->getEndPoint(/*emulateCCW=*/true).y
                );
                dirVec = Base::Vector3d(0.f, 0.f, 1.0)
                    % (arcSeg->getEndPoint(/*emulateCCW=*/true) - arcSeg->getCenter());
            }
        }
        dirVec.Normalize();
    }
	void DrawSketchHandlerLineSet::onKeyRelease(const std::string& key)
	{
        if (key == "CONTROL_L") {
            ctrlDown = false;
        }
        if (key == "ESCAPE") {
            quit();
        }
	}
}