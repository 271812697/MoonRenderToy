#include "Sketcher/SketcherObj.h"
#include "editor/Toolbar/sketchToolbar.h"
#include "Geometry.h"
#include "renderer/SceneView.h"
#include "Interactive/Im3DRenderer.h"
#include "Interactive/Widgets/DrawSketchHandler.h"
#include "Core/Global/ServiceLocator.h"
#include "core/ViewTool.h"
#include "Sketcher/SketcheTool2D.h"
#include "base/Tools.h"
#include "core/log.h"
#include "Qtimgui/imgui/imgui.h"
#include "Qtimgui/implot/implotCustom.h"
#include <QInputDialog>
#include <algorithm>
#include <chrono>
#include <cstdio>
#include <TopoDS.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
#include <ShapeFix_Wire.hxx>
#include <BRep_Builder.hxx>
#include <GeomAPI.hxx>
#include <Geom2dAPI_InterCurveCurve.hxx>
#include <Geom2dAPI_ProjectPointOnCurve.hxx>
namespace MOON {
    static double pointToSegmentDist(const Base::Vector3d& p, const Base::Vector3d& s, const Base::Vector3d& e, double& u) {
        Base::Vector3d se = e - s;
        Base::Vector3d sp = p - s;
        double t = sp.Dot(se) / se.Dot(se);
        if (t < 0.0) {
            u = 0.0;
            return sp.Length();
        }

        if (t > 1.0) {
            u = 1.0;
            return (p - e).Length();
        }
        u = t;
        Base::Vector3d proj = s + t * se;
        return (p - proj).Length();
        };
    static bool areParamsWithinApproximation(double param1, double param2)
    {
        // From testing: 500x (or 0.000050) is needed in order to not falsely distinguish points
        // calculated with seekTrimPoints
        return (std::abs(param1 - param2) < Precision::PApproximation());
    }
    static bool arePointsWithinPrecision(const Base::Vector3d& point1, const Base::Vector3d& point2)
    {
        // From testing: 500x (or 0.000050) is needed in order to not falsely distinguish points
        // calculated with seekTrimPoints
        return ((point1 - point2).Length() < 500 * Precision::Confusion());
    }
    // Constraint types that get a numeric viewport label.
    static bool isDimensionLabelType(Sketcher::ConstraintType type)
    {
        return type == Sketcher::ConstraintType::Distance
            || type == Sketcher::ConstraintType::DistanceX
            || type == Sketcher::ConstraintType::DistanceY
            || type == Sketcher::ConstraintType::Radius
            || type == Sketcher::ConstraintType::Diameter
            || type == Sketcher::ConstraintType::Angle;
    }
    // Default screen-space offset (in pixels) applied to the auto anchor when
    // the user has not moved the label manually.
    static void defaultLabelOffsetPx(const Sketcher::Constraint* c, float& dx, float& dy)
    {
        dx = 0.0f;
        dy = -22.0f;  // by default put the text above the measured geometry
        if (!c) {
            return;
        }
        if (c->Type == Sketcher::ConstraintType::DistanceY) {
            dx = 26.0f;  // vertical distances read better on the right side
            dy = 0.0f;
        }
        else if (c->Type == Sketcher::ConstraintType::Radius
            || c->Type == Sketcher::ConstraintType::Diameter) {
            dx = 22.0f;  // diagonal offset away from the center/rim
            dy = -22.0f;
        }
    }
	SketcherObj::SketcherObj() :EventWidget("SketcherObj")
    {
        setActive(true);
    }
    SketcherObj::~SketcherObj()
    {
        for (Sketcher::Constraint* c : mConstraintList) {
            delete c;
        }
        mConstraintList.clear();
    }
    void SketcherObj::onUpdate()
    {
		isHaveActiveHandler = false;
		auto& gizmoWidgets =renderer->getGizmoWidgets();
        for (auto& it : gizmoWidgets) {
			if (it.second->isActived() && dynamic_cast<DrawSketchHandler*>(it.second)) {
				isHaveActiveHandler = true;
                break;
			}
        }
        if (isHaveActiveHandler) {
            // A geometry-drawing tool takes over the mouse; stop any label
            // hover/drag so it cannot fight with the active handler.
            m_labelHover = -1;
            m_labelDrag = -1;
        }
        draw();
    }
    void SketcherObj::onMouseMove()
    {
        if (!isHaveActiveHandler && isInEdit) {
            updateConstraintLabelInteraction();
            // While the cursor rests on a dimension label (or drags one) the
            // mouse must not select/move the geometry underneath it.
            if (m_labelDrag >= 0 || m_labelHover != -1) {
                preSelectGeoId = { -1, PointPos::none };
                return;
            }
        }
        onSketchPosP2 = onSketchPosMove;
		Base::Vector2d preOnSketchPosMove = onSketchPosMove;
        if (!isHaveActiveHandler&& isInEdit) {
            pickGeo();
            if (selectState == Stop&& preSelectGeoId.GeoId!=-1) {
                selectState = Hot;
            }
            else if(selectState== OperationGeo) {
                // Single circle/arc body drag: the solver anchors the center
                // and the mouse sets the rim point (radius). Everything else
                // (endpoints, centers, whole lines, group drag) is moved by
                // relative displacement.
                bool radiusDrag = selectIds.size() == 1 && selectIds[0].pointPos == PointPos::none;
                if (radiusDrag) {
                    Part::Geometry* geo = getGeometry(selectIds[0].GeoId);
                    radiusDrag = geo
                        && (geo->is<Part::GeomCircle>() || geo->is<Part::GeomArcOfCircle>());
                }

                std::vector<Sketcher::GeoElementId> dragIds;
                dragIds.reserve(selectIds.size());
                for (const auto& sel : selectIds) {
                    dragIds.emplace_back(sel.GeoId, sel.pointPos);
                }
                if (!m_dragSolverInit) {
                    solvedSketch.resetInitMove();
                    m_dragSolverInit = solvedSketch.initMove(dragIds) == 0;
                }
                if (m_dragSolverInit) {
                    Base::Vector3d moveTo;
                    bool relative = true;
                    if (radiusDrag) {
                        moveTo = Base::Vector3d(onSketchPosMove.x, onSketchPosMove.y, 0.0);
                        relative = false;  // rim follows the mouse, center stays
                    }
                    else {
                        const Base::Vector2d totalDelta = onSketchPosMove - onSketchPosP1;
                        moveTo = Base::Vector3d(totalDelta.x, totalDelta.y, 0.0);
                    }
                    const int status = solvedSketch.moveGeometries(dragIds, moveTo, relative);
                    if (status == 0) {
                        for (auto& geo : mGeoList) {
                            mGeoSegment.erase(geo.get());
                        }
                        mGeoList.clear();
                        std::vector<Part::Geometry*> geomlist = solvedSketch.extractGeometry();
                        for (Part::Geometry* geo : geomlist) {
                            addGeometry(geo);
                        }
                        for (Part::Geometry* geo : geomlist) {
                            delete geo;
                        }
                    }
                    return;  // solver path already handled this frame
                }

                bool solveS = false;
                for (int i = 0; i < selectIds.size(); i++) {
                    moveGeo(
                        selectIds[i],
                        onSketchPosMove.x - preOnSketchPosMove.x,
                        onSketchPosMove.y - preOnSketchPosMove.y
                    );
                    solveS = true;
                }
                if (solveS) {
                    solve();
                }
            }
            else if (selectState ==Hot&& preSelectGeoId.GeoId == -1) {
                selectState = Stop;
            }
        }
    }
    bool sketchDrawRect = false;
    void SketcherObj::onLeftMousePressed()
    {
        if (!isHaveActiveHandler && isInEdit) {
            auto [mx, my] = m_sceneView->getInutState().GetMousePosition();
            const int labelHit = pickConstraintLabelAt(static_cast<float>(mx), static_cast<float>(my));
            if (labelHit >= 0) {
                const Sketcher::Constraint* c = getConstraint(labelHit);
                float defDx = 0.0f, defDy = 0.0f;
                defaultLabelOffsetPx(c, defDx, defDy);
                const auto it = c ? m_labelManualOffsetPx.find(c) : m_labelManualOffsetPx.end();
                m_labelDragOffsetPx = (it != m_labelManualOffsetPx.end())
                    ? it->second
                    : Base::Vector2d(defDx, defDy);
                m_labelDrag = labelHit;
                m_labelDragPressPx = Base::Vector2d(mx, my);
                clearSelect();
                selectState = Stop;
                preSelectGeoId = { -1, PointPos::none };

                const auto now = std::chrono::steady_clock::now();
                const bool isDoubleClick = m_lastLabelClick == labelHit
                    && std::chrono::duration_cast<std::chrono::milliseconds>(now - m_lastLabelClickTime).count()
                    < 400;
                if (isDoubleClick) {
                    m_lastLabelClick = -1;
                    m_labelDrag = -1;
                    editConstraintValue(labelHit);
                }
                else {
                    m_lastLabelClick = labelHit;
                    m_lastLabelClickTime = now;
                }
                return;
            }
            m_lastLabelClick = -1;
        }
        sketchDrawRect = true;
        onSketchPosP1=getMouseHitSketchPlanePoint();
        onSketchPosClicked = onSketchPosP1;
        m_dragSolverInit = false;
		if (preSelectGeoId.GeoId == -1) {
            pickGeo();
		}
        if (!isHaveActiveHandler) {
            if (selectState == Hot) {
                if (preSelectGeoId.GeoId != -1) {
                    //
                    if (selectMode == OverrideSelect) {
                        clearSelect();
                    }
                    addSelect(preSelectGeoId);
                    selectState = OperationGeo;
                }
                else
                {
                    selectState = Stop;
                }
            }
		    else if(selectState == Stop) {
			    selectState = DragRect;
		    }
        }
    }
    void SketcherObj::onLeftMouseReleased()
    {
        if (m_labelDrag >= 0) {
            // A dimension label was being dragged; the label position is kept
            // in m_labelManualOffsetPx, so simply end the drag here.
            m_labelDrag = -1;
            return;
        }
        sketchDrawRect = false;
        onSketchPosP2 = getMouseHitSketchPlanePoint();
        if (!isHaveActiveHandler) {
            if (selectState == OperationGeo) {
                selectState = Hot;
                m_dragSolverInit = false;
                solvedSketch.resetInitMove();
            }
            else if (selectState == DragRect)  {
                if (selectMode == OverrideSelect) {
                    clearSelect();
                }
			    Base::Vector2d minPt(std::min(onSketchPosP1.x, onSketchPosP2.x), std::min(onSketchPosP1.y, onSketchPosP2.y));
			    Base::Vector2d maxPt(std::max(onSketchPosP1.x, onSketchPosP2.x), std::max(onSketchPosP1.y, onSketchPosP2.y));
			    for (int i = 0;i < mGeoList.size();i++) {
				    auto& seg = mGeoSegment[mGeoList[i].get()];
				    bool isInside = true;
				    for (int j = 0;j < seg.point.size();j++) {
                        bool flag = seg.point[j].x >= minPt.x && seg.point[j].x <= maxPt.x
                            && seg.point[j].y >= minPt.y && seg.point[j].y <= maxPt.y;
					    if (!flag) {
                            isInside = false;
						    break;
					    }
				    }
                    if (isInside) {
                        addSelect({i,PointPos::none});
                    }
                    else
                    {
                        for (int j = 0; j < seg.sepoints.size(); j++) {
                            bool flag = seg.sepoints[j].coord.x >= minPt.x && seg.sepoints[j].coord.x <= maxPt.x
                                && seg.sepoints[j].coord.y >= minPt.y && seg.sepoints[j].coord.y <= maxPt.y;
                            if (flag) {
                                addSelect({ i,seg.sepoints[j].pointPos });
                                break;
                            }
                        }
                    }
			    }
                selectState = Stop;
            }
        }
    }
    void SketcherObj::onKeyPress(const std::string& key)
    {
        if (key == "DELETE"&& !isHaveActiveHandler) {
            std::vector<int>deletList(selectIds.size());
            for (int i = 0; i < selectIds.size(); i++) {
                deletList[i] = selectIds[i].GeoId;
            }
            deleteGeometries(deletList);
            selectIds.clear();
            solve();
        }
        else if (key == "CONTROL_L") {
            selectMode = AppendSelect;
        }
    }
    void SketcherObj::onKeyRelease(const std::string& key)
    {
        if (key == "CONTROL_L") {
            selectMode = OverrideSelect;
        }
    }
    void SketcherObj::setPlane(const SketcherPlane2D& plane)
    {
        mPlane = plane;     
        fitCamera();
        //GetService(SketchToolbar).disableAllHandlers();
    }
    void SketcherObj::fitCamera()
    {
        
        auto& view = GetService(Editor::Panels::SceneView);
        view.GetCameraController().EnableRotate(false);
        view.GetCamera()->SetSize(100);
        view.GetCamera()->SetProjectionMode(Rendering::Settings::EProjectionMode::ORTHOGRAPHIC);
        float pos = view.GetCamera()->GetFar() / 2.0;
        Maths::FVector3 normal(mPlane.normal.x, mPlane.normal.y, mPlane.normal.z);
        Maths::FVector3 up(mPlane.yAxis.x, mPlane.yAxis.y, mPlane.yAxis.z);
        Maths::FQuaternion quat = Maths::FQuaternion::LookAt(-normal, up);
        view.GetCameraController().MoveToPose(Maths::FVector3(mPlane.origin.x, mPlane.origin.y, mPlane.origin.z) + normal * pos, quat);
        planeTransform = updateTransform();
    }
    void SketcherObj::beginEdit()
    {
       isInEdit = true;
       setActive(true);
       fitCamera();
    }
    SketcherPlane2D SketcherObj::getPlane()
    {
        return mPlane;
    }
    void SketcherObj::getPlaneNormal(double* p)
    {       
        p[0] = mPlane.normal.x;
		p[1] = mPlane.normal.y;
        p[2] = mPlane.normal.z;
    }
    void SketcherObj::drawBackground()
    {
        if (!InEdit()) return;

        // Adaptive background grid: big cells use the darker tone, the smaller
        // subdivisions inside use the lighter tone. The spacing snaps to a nice
        // 1/2/5 x 10^n step so the on-screen density stays roughly constant while
        // zooming, and each line spans the whole visible sketch region.
        if (m_drawGrid) {
            const Eigen::Vector4<uint8_t> minorColor(150, 132, 118, 118); // (A,B,G,R) = (255, b,g,r)
            const Eigen::Vector4<uint8_t> majorColor(255, 66, 56, 56); // (A,B,G,R) = (255, b,g,r)
            const auto* gridCam = m_sceneView->GetCamera();
            if (gridCam) {
                const auto& gProj = gridCam->GetProjectionMatrix();
                const float gProj11 = gProj(1, 1);
                const auto& gFd = m_sceneView->GetRenderer().GetFrameDescriptor();
                const float gScreenH = static_cast<float>(gFd.renderHeight);
                const float gScreenW = static_cast<float>(gFd.renderWidth);
                const float gAspect = gScreenH > 0.0f ? gScreenW / gScreenH : 1.0f;
                if (gProj11 > 0.0f && gScreenH > 0.0f &&
                    gridCam->GetProjectionMode() == ::Rendering::Settings::EProjectionMode::ORTHOGRAPHIC) {
                    const float halfH = 1.0f / gProj11;
                    const float hx = halfH * gAspect;
                    const float hy = halfH;
                    // Visible sketch-space rectangle: project the four viewport corners
                    // onto the sketch plane (view z is irrelevant for an ortho camera).
                    const auto& gView = gridCam->GetViewMatrix();
                    const Maths::FMatrix4 gInvView = Maths::FMatrix4::Inverse(gView);
                    const float xs[2] = { -hx, hx };
                    const float ys[2] = { -hy, hy };
                    float uMin = 1e30f, uMax = -1e30f, vMin = 1e30f, vMax = -1e30f;
                    for (int i = 0; i < 2; ++i) {
                        for (int j = 0; j < 2; ++j) {
                            const Maths::FVector3 wp = gInvView.MulPoint(Maths::FVector3(xs[i], ys[j], 0.0f));
                            const Base::Vector3d d(wp.x - mPlane.origin.x, wp.y - mPlane.origin.y, wp.z - mPlane.origin.z);
                            const float u = static_cast<float>(d.Dot(mPlane.xAxis));
                            const float v = static_cast<float>(d.Dot(mPlane.yAxis));
                            uMin = std::min(uMin, u); uMax = std::max(uMax, u);
                            vMin = std::min(vMin, v); vMax = std::max(vMax, v);
                        }
                    }
                    // Nice step: smallest 1/2/5 x 10^n candidate above the target so the
                    // minor lines are roughly 40 px apart on screen.
                    const float targetStep = 40.0f * (2.0f * halfH) / gScreenH;
                    const float mag = std::pow(10.0f, std::floor(std::log10(std::max(targetStep, 1e-6f))));
                    float step = mag;
                    if (step < targetStep) step = 2.0f * mag;
                    if (step < targetStep) step = 5.0f * mag;
                    if (step < targetStep) step = 10.0f * mag;
                    const int i0 = static_cast<int>(std::ceil(uMin / step));
                    const int i1 = static_cast<int>(std::floor(uMax / step));
                    for (int i = i0; i <= i1; ++i) {
                        const float g = i * step;
                        const auto& col = (i % 5) == 0 ? majorColor : minorColor;
                        renderer->drawLine(mPlane.valueEigen(g, vMin), mPlane.valueEigen(g, vMax), 1.0f, col);
                    }
                    const int j0 = static_cast<int>(std::ceil(vMin / step));
                    const int j1 = static_cast<int>(std::floor(vMax / step));
                    for (int j = j0; j <= j1; ++j) {
                        const float g = j * step;
                        const auto& col = (j % 5) == 0 ? majorColor : minorColor;
                        renderer->drawLine(mPlane.valueEigen(uMin, g), mPlane.valueEigen(uMax, g), 1.0f, col);
                    }
                }
                else {
                    // Fallback for non-orthographic views: fixed extent grid.
                    const float extent = 100.0f;
                    for (int k = -10; k <= 10; ++k) {
                        const float g = k * 10.0f;
                        const auto& col = (k % 5) == 0 ? majorColor : minorColor;
                        renderer->drawLine(mPlane.valueEigen(g, -extent), mPlane.valueEigen(g, extent), 1.0f, col);
                        renderer->drawLine(mPlane.valueEigen(-extent, g), mPlane.valueEigen(extent, g), 1.0f, col);
                    }
                }
            }
        }

        // Infinite X/Y axes: intersect each axis line with the viewport rectangle
        // and draw the segment between the two crossings so they span the screen.
        renderer->pushSize(3);
        const auto* axisCam = m_sceneView->GetCamera();
        if (axisCam) {
            const auto& proj = axisCam->GetProjectionMatrix();
            const float proj11 = proj(1, 1);
            const auto& fd = m_sceneView->GetRenderer().GetFrameDescriptor();
            const float screenH = static_cast<float>(fd.renderHeight);
            const float aspect = screenH > 0.0f ? static_cast<float>(fd.renderWidth) / screenH : 1.0f;
            if (proj11 > 0.0f && axisCam->GetProjectionMode() == ::Rendering::Settings::EProjectionMode::ORTHOGRAPHIC) {
                const auto& viewM = axisCam->GetViewMatrix();
                const Maths::FVector3 origin3(mPlane.origin.x, mPlane.origin.y, mPlane.origin.z);
                const Maths::FVector3 v0 = viewM.MulPoint(origin3);
                const float halfH = 1.0f / proj11;
                const float hx = halfH * aspect;
                const float hy = halfH;
                const Base::Vector3d dirs[2] = { mPlane.xAxis, mPlane.yAxis };
                const Eigen::Vector4<uint8_t> colors[2] = { {255, 0, 0, 255}, {255, 0, 255, 0} };
                for (int a = 0; a < 2; ++a) {
                    const Maths::FVector3 p1 = viewM.MulPoint(Maths::FVector3(
                        mPlane.origin.x + dirs[a].x, mPlane.origin.y + dirs[a].y, mPlane.origin.z + dirs[a].z));
                    const Maths::FVector3 dv(p1.x - v0.x, p1.y - v0.y, 0.0f);
                    const float coords[2] = { v0.x, v0.y };
                    const float dirC[2] = { dv.x, dv.y };
                    const float half[2] = { hx, hy };
                    float lo = -1e9f, hi = 1e9f;
                    bool crosses = true;
                    for (int c = 0; c < 2; ++c) {
                        if (std::abs(dirC[c]) < 1e-9f) {
                            if (std::abs(coords[c]) > half[c]) { crosses = false; break; }
                        }
                        else {
                            const float t1 = (-half[c] - coords[c]) / dirC[c];
                            const float t2 = (half[c] - coords[c]) / dirC[c];
                            lo = std::max(lo, std::min(t1, t2));
                            hi = std::min(hi, std::max(t1, t2));
                        }
                    }
                    if (crosses && hi >= lo) {
                        const float ext = std::max((hi - lo) * 0.05f, 1e-3f);
                        renderer->pushColor(colors[a]);
                        if (a == 0) {
                            renderer->drawLine(mPlane.valueEigen(lo - ext, 0.0), mPlane.valueEigen(hi + ext, 0.0));
                        }
                        else {
                            renderer->drawLine(mPlane.valueEigen(0.0, lo - ext), mPlane.valueEigen(0.0, hi + ext));
                        }
                        renderer->popColor();
                    }
                }
            }
            else {
                renderer->pushColor({ 255,0,0,255 });
                renderer->drawLine(mPlane.valueEigen(500,0), mPlane.valueEigen(-500,0));
                renderer->popColor();
                renderer->pushColor({255,0,255,0});
                renderer->drawLine(mPlane.valueEigen(0, 500), mPlane.valueEigen(0, -500));
                renderer->popColor();
            }
        }
        renderer->popSize();

        // Origin marker.
        renderer->pushColor({ 255,255,255,0 });
        renderer->drawPoint(mPlane.valueEigen(0,0));
        renderer->popColor();
    }

    void SketcherObj::draw() {
        if (InEdit()) {
            drawBackground();
        }
        renderer->pushSize(3);
        if (selectState == DragRect && sketchDrawRect&&!isHaveActiveHandler) {
            Eigen::Vector3f p1 = mPlane.valueEigen(Base::Vector2d(std::min(onSketchPosP1.x, onSketchPosP2.x), std::min(onSketchPosP1.y, onSketchPosP2.y)));
            Eigen::Vector3f p2 = mPlane.valueEigen(Base::Vector2d(std::max(onSketchPosP1.x, onSketchPosP2.x), std::min(onSketchPosP1.y, onSketchPosP2.y)));
            Eigen::Vector3f p3 = mPlane.valueEigen(Base::Vector2d(std::max(onSketchPosP1.x, onSketchPosP2.x), std::max(onSketchPosP1.y, onSketchPosP2.y)));
            Eigen::Vector3f p4 = mPlane.valueEigen(Base::Vector2d(std::min(onSketchPosP1.x, onSketchPosP2.x), std::max(onSketchPosP1.y, onSketchPosP2.y)));
            renderer->drawQuad(p1,p2,p3,p4);
        }
        Eigen::Vector4<uint8_t> pointColor(255, 0, 0, 255);
        Eigen::Vector4<uint8_t> preselectColor(255, 0, 255, 255);
        Eigen::Vector4<uint8_t> selectColor(255, 255, 255, 0);
        float pointSize = 12;
        for (auto& it: mGeoSegment) {
            auto& sePoints = it.second.sepoints;
            for (int i = 0;i < sePoints.size();i++) {
                renderer->drawPoint(mPlane.valueEigen(sePoints[i].coord.x, sePoints[i].coord.y), pointSize+1, pointColor);
            }
        }  
        for (int i = 0;i < mGeoList.size();i++) {
			bool isSelect = false;
			for (int j = 0;j < selectIds.size();j++) {
				if (selectIds[j].GeoId == i) {
                    if (selectIds[j].pointPos == PointPos::none) {
                        isSelect = true;
                    }
                    else
                    {
                        auto &segment=mGeoSegment[mGeoList[i].get()];
                        for (int k = 0; k < segment.sepoints.size(); k++) {
                            if (segment.sepoints[k].pointPos == selectIds[j].pointPos) {
                                renderer->drawPoint(mPlane.valueEigen(segment.sepoints[k].coord.x, segment.sepoints[k].coord.y), pointSize, selectColor);
                            }
                        }
                    }
				}
			}
			if (isSelect) {
				renderer->pushColor(selectColor);
			}
			else if (i == preSelectGeoId.GeoId&&selectState!= OperationGeo) {
				renderer->pushColor(preselectColor);
			}
			else {
				renderer->pushColor(Eigen::Vector4<uint8_t>(255, 0, 0, 0));
			}
			auto& geo = mGeoList[i];
			if (geo->isDerivedFrom<Part::GeomCurve>()) {
                auto& seg = mGeoSegment[geo.get()];
                for (int i = 0;i < seg.point.size() - 1;i++) {
                    renderer->drawLine(mPlane.valueEigen(seg.point[i].x, seg.point[i].y), mPlane.valueEigen(seg.point[i+1].x, seg.point[i+1].y));
                }
            }
			renderer->popColor();
        }
        renderer->popSize();
        drawConstraintLabels();
    }
    bool SketcherObj::InEdit() const
    {
        return isInEdit;
    }
    void SketcherObj::makeDone()
    {
        isInEdit = false;
        auto& view = GetService(Editor::Panels::SceneView);
        view.GetCameraController().EnableRotate(true);
        doneWireShape = toShape();
		doneFaceShape = doneWireShape.makeElementFace(nullptr, "Part::FaceMakerBullseye");
        GetService(SketchToolbar).disableAllHandlers();
    }
    int SketcherObj::solve(bool updateGeoAfterSolving)
    {
        //Reset
        solvedSketch.resetInitMove();
        //Set Up geometry and contraint
        std::vector<Part::Geometry*> GeoList;
        for (int i = 0; i < mGeoList.size(); i++) {
            GeoList.push_back(mGeoList[i].get());
        }
        lastDoF=solvedSketch.setUpSketch(
            GeoList, mConstraintList,0);
        //restrive the solver information
        retrieveSolverDiagnostics();

        lastSolverStatus = GCS::Failed;
        int err = 0;
        if (lastHasRedundancies) {// redundant constraints
            err = -2;
        }
        if (lastDoF < 0) {// over-constrained sketch
            err = -4;
        }
        else if (lastHasConflict) {// conflicting constraints
            // The situation is exactly the same as in the over-constrained situation.
            err = -3;
        }
        else if (lastHasMalformedConstraints) {
            err = -5;
        }
        else {
            lastSolverStatus = solvedSketch.solve();
            if (lastSolverStatus != 0) {// solving
                err = -1;
            }
        }
        if (err==0) {
            // Replace the geometry in place. FreeCAD keeps the geometry
            // property list untouched when there is no change; here we rebuild
            // the internal list directly and never route through
            // deleteGeometries() (that would wipe constraints referencing the
            // very elements we just solved).
            for (auto& geo : mGeoList) {
                mGeoSegment.erase(geo.get());
            }
            mGeoList.clear();
            std::vector<Part::Geometry*> geomlist = solvedSketch.extractGeometry();
            for (Part::Geometry* geo : geomlist) {
                addGeometry(geo);  // copies into owned storage
            }
            for (Part::Geometry* geo : geomlist) {
                delete geo;        // extractGeometry() hands out clones
            }
        }
        return err;
    }
    int SketcherObj::addGeometry(std::unique_ptr<Part::Geometry>& ptr)
    {
        Part::Geometry* geo = ptr.get();
        mGeoSegment[geo]=getCurveSegment(geo);
        mGeoList.push_back(std::move(ptr));
        return mGeoList.size() - 1;
    }
    int SketcherObj::addGeometry(Part::Geometry* curve)
    {
        std::unique_ptr<Part::Geometry>temp(curve->copy());
        return addGeometry(temp);
    }
    void SketcherObj::addGeometry(const std::vector<Part::Geometry*>& curveList)
    {
        for (int i = 0; i < curveList.size(); i++) {
            std::unique_ptr<Part::Geometry> temp(curveList[i]->copy());
            addGeometry(temp);
        }
    }
    Part::Geometry* SketcherObj::getGeometry(int GeoId)
    {
		if (GeoId >= 0 && GeoId < mGeoList.size()) {
			return mGeoList[GeoId].get();
		}
        return nullptr;
    }
    const Part::Geometry* SketcherObj::getGeometry(int GeoId) const
    {
        if (GeoId >= 0 && GeoId < static_cast<int>(mGeoList.size())) {
            return mGeoList[GeoId].get();
        }
        return nullptr;
    }
    int SketcherObj::getHighestCurveIndex()
    {
        return mGeoList.size()-1;
    }
    int SketcherObj::getPickGeoIndex(const Base::Vector2d& pos,const Base::Matrix4D& mat)
    {
    
        Base::Matrix4D trans = mat * planeTransform;
        Base::Vector3d p1 = trans * Base::Vector3d(pos.x, pos.y, 0);

        int ret = -1;
        double deltaTole = 15.0;
        double minDist = 10000.0;
        // 遍历所有几何图元
        for (int i = 0; i < mGeoList.size(); i++) {
            Part::Geometry* geo = mGeoList[i].get();
            if (geo->isDerivedFrom<Part::GeomCurve>()) {
                auto& segment = mGeoSegment[geo];
                int segCount = segment.point.size();
                if (segCount < 2)
                    continue;
                // 遍历每一段线段 [k] → [k+1]
                for (int k = 0; k < segCount - 1; k++) {
                    Base::Vector3d s = segment.point[k];
                    Base::Vector3d e = segment.point[k + 1];
                    // ✅ 使用 Lambda 计算真正的点到线段距离
                    double u;
                    double dist = pointToSegmentDist(p1, trans*s, trans*e,u);

                    if (dist < deltaTole && dist < minDist) {
                        minDist = dist;
                        ret = i;
                    }
                }
            }
        }
        return ret;
    }
    SketcherObj::SelectGeoId SketcherObj::testSelect(const Base::Vector2d& pos)
    {
        Maths::FMatrix4 mat = m_sceneView->GetCamera()->GetViewPortMatrix();
        Base::Matrix4D viewPortMat(
            mat.data[0], mat.data[1], mat.data[2], mat.data[3],
            mat.data[4], mat.data[5], mat.data[6], mat.data[7],
            mat.data[8], mat.data[9], mat.data[10], mat.data[11],
            mat.data[12], mat.data[13], mat.data[14], mat.data[15]
        );
        Base::Matrix4D trans = viewPortMat * getplaneTransform();
        Base::Vector3d p1 = trans * Base::Vector3d{ pos.x,pos.y,0.0 };
        double deltaTole = 5.0;
        double minDist = 10000.0;
        SelectGeoId ret = {-1,PointPos::none } ;   

        // travel all segments
        for (int i = 0; i < mGeoList.size(); i++) {
            Part::Geometry* geo = mGeoList[i].get();
            auto& segment = mGeoSegment[geo];
            for (int j = 0; j < segment.sepoints.size(); j++) {
                double dist = (p1 - trans * segment.sepoints[j].coord).Length();
                if (dist < deltaTole && dist < minDist) {
                    minDist = dist;
                    ret.GeoId = i;
                    ret.pointPos = segment.sepoints[j].pointPos;
                }
            }
        }
        if (ret.GeoId == -1) {
            for (int i = 0; i < mGeoList.size(); i++) {
                Part::Geometry* geo = mGeoList[i].get();
                auto& segment = mGeoSegment[geo];
                if (geo->isDerivedFrom<Part::GeomCurve>()) {
                    for (int j = 0; j < segment.point.size() - 1; j++) {
                        double u = 0.0;
                        double dist = pointToSegmentDist(
                            p1,
                            trans * segment.point[j],
                            trans * segment.point[j + 1],
                            u);

                        if (dist < deltaTole && dist < minDist) {
                            minDist = dist;
                            ret.GeoId = i;
                        }
                    }
                }
                else if (geo->is<Part::GeomPoint>())
                {
                    Base::Vector3d pp = static_cast<Part::GeomPoint*>(geo)->getPoint();
                    double dist = (p1 - trans * pp).Length();
                    if (dist < deltaTole && dist < minDist) {
                        minDist = dist;
                        ret.GeoId = i;
                    }
                }
            }
        }
        return ret;
    }
    std::vector<int> SketcherObj::getSelectIds() const
    {
        std::vector<int>selectIdLists(selectIds.size());
        for (int i = 0; i < selectIds.size(); i++) {
            selectIdLists[i] = selectIds[i].GeoId;
        }
        return selectIdLists;
    }

    void SketcherObj::addSelect(int id)
    {
        addSelect({id,PointPos::none});
    }

    void SketcherObj::removeSelect(const std::vector<int>& idList)
    {
        int left = 0;
        for (int right = 0; right < selectIds.size();right++) {
            bool removeFlag = false;
            for (int i = 0; i < idList.size(); i++) {
                if (selectIds[right].GeoId == idList[i]) {
                    removeFlag = true;
                    break;
                }
            }     
            if (!removeFlag) {
                selectIds[left++] = selectIds[right];
            }
        }
        selectIds.resize(left);
    }
    bool SketcherObj::snapPoint(Base::Vector2d& pos, const std::set<int>& avoid)
    {
        Maths::FMatrix4 mat = m_sceneView->GetCamera()->GetViewPortMatrix();
        Base::Matrix4D pla(
            mat.data[0], mat.data[1], mat.data[2], mat.data[3],
            mat.data[4], mat.data[5], mat.data[6], mat.data[7],
            mat.data[8], mat.data[9], mat.data[10], mat.data[11],
            mat.data[12], mat.data[13], mat.data[14], mat.data[15]
        );
        Base::Matrix4D trans= pla*getplaneTransform();
        //get the screen pos
        Base::Vector3d screenpPos = trans*Base::Vector3d{pos.x,pos.y,0.0};
        double deltaTole = 10.0;
        double minDist = 10000.0;
        bool ret = false;
        // travel all segments
        for (int i = 0; i < mGeoList.size(); i++) {
            if (!avoid.count(i)) {
                Part::Geometry* geo = mGeoList[i].get();  
                auto& segment = mGeoSegment[geo];
                for (int j = 0;j < segment.sepoints.size();j++) {
				    double dist = (screenpPos -trans*segment.sepoints[j].coord).Length();
				    if (dist < deltaTole && dist < minDist) {
					    minDist = dist;
					    ret = true;
					    pos = { segment.sepoints[j].coord.x, segment.sepoints[j].coord.y };
				    }
                }
            }

        }
        if (!ret) {
            //snap to orgin or XAxis or YAxis
            Base::Vector3d screenOrigin = trans * Base::Vector3d(0, 0, 0);
            double dist = (screenpPos - screenOrigin).Length();
            if (dist < deltaTole) {
                pos = { 0.0, 0.0 };
                return true;
            }
            double deltaX = abs(screenpPos.x - screenOrigin.x);
            double deltaY = abs(screenpPos.y - screenOrigin.y);
            if (deltaX < deltaTole && deltaX < deltaY) {
                pos.x = 0.0;
                return true;
            }
            if (deltaY < deltaTole && deltaY < deltaX) {
                pos.y = 0.0;
                return true;
            }
            //snap to curve
            for (int i = 0; i < mGeoList.size(); i++) {
                if (!avoid.count(i)) {
                    Part::Geometry* geo = mGeoList[i].get();
                    auto& segment = mGeoSegment[geo];
                    if (geo->isDerivedFrom<Part::GeomCurve>()) {
                        for (int j = 0;j < segment.point.size()-1;j++) {
                            double u = 0.0;
                            double dist = pointToSegmentDist(
                                screenpPos,
                                trans * segment.point[j],
                                trans * segment.point[j+1],
                                u);
                            if (dist < deltaTole && dist < minDist) {
                                minDist = dist;
                                ret = true;
                                u= segment.params[j] +u*(segment.params[j+1]-segment.params[j]);
							    Base::Vector3d pp=static_cast<Part::GeomCurve*>(geo)->value(u);
                                pos = { pp.x, pp.y };
                            }
                        }   
                    }
                }
            }
        }
        return ret;
    }
    int SketcherObj::fillet(int GeoId1, int GeoId2, const Base::Vector3d& refPnt1, const Base::Vector3d& refPnt2, double radius, bool trim, bool createCorner, bool chamfer)
    {
        if (GeoId1 < 0 || GeoId1 > getHighestCurveIndex() || GeoId2 < 0 || GeoId2 > getHighestCurveIndex()) {
            return -1;
        }
        // If either of the two input lines are locked, don't try to trim since it won't work anyway
        Part::Geometry* geo1 = getGeometry(GeoId1);
        Part::Geometry* geo2 = getGeometry(GeoId2);
        int pos1 = 0;
        int pos2 = 0;
        bool reverse = false;
        std::unique_ptr<Part::GeomArcOfCircle> arc(createFilletGeometry(geo1, geo2, refPnt1, refPnt2, radius, pos1, pos2, reverse));
        if (!arc) {
            return -1;
        }

        int filletId = addGeometry(arc.get());
        if (filletId < 0) {
            return -1;
        }

        int PosId1 = static_cast<int>(pos1);
        int PosId2 = static_cast<int>(pos2);
        int filletPosId1 = -1;
        int filletPosId2 = -1;

        Base::Vector3d p1 = arc->getStartPoint(true);
        Base::Vector3d p2 = arc->getEndPoint(true);

        if (trim) {
            //if (reverse) {
            //    moveGeometry(GeoId1, PosId1, p1, false, true);
            //    moveGeometry(GeoId2, PosId2, p2, false, true);
            //}
            //else {
            //    moveGeometry(GeoId1, PosId1, p2, false, true);
            //    moveGeometry(GeoId2, PosId2, p1, false, true);
            //}
            auto* line1 = static_cast<Part::GeomLineSegment*>(geo1);
            auto* line2 = static_cast<Part::GeomLineSegment*>(geo2);

            auto s1= line1->getStartPoint();
            auto e1 = line1->getEndPoint();
            auto s2 = line2->getStartPoint();
            auto e2 = line2->getEndPoint();
           if (reverse) {
               if (PosId1 == 1) {//>0
                   line1->setPoints(p1,e1);
               }
               else if(PosId1==2)
               {
                   line1->setPoints(s1, p1);
               }
               if (PosId2 == 1) {//>0
                   line2->setPoints(p2, e2);
               }
               else if (PosId2 == 2)
               {
                   line2->setPoints(s2, p2);
               }
            }
            else {
               if (PosId1 == 1) {//>0
                   line1->setPoints(p2, e1);
               }
               else if (PosId1 == 2)
               {
                   line1->setPoints(s1, p2);
               }
               if (PosId2 == 1) {//>0
                   line2->setPoints(p1, e2);
               }
               else if (PosId2 == 2)
               {
                   line2->setPoints(s2, p1);
               }
            }
           updateGeoSegment(GeoId1);
           updateGeoSegment(GeoId2);
        }

        if (chamfer) {
            auto line = std::make_unique<Part::GeomLineSegment>();
            line->setPoints(p1, p2);
            int lineGeoId = addGeometry(line.get());
        }
        return 0;
    }
    bool SketcherObj::seekTrimPoints(int geometryIndex,
        const Base::Vector3d& point,
        int& geometryIndex1,
        Base::Vector3d& intersect1,
        int& geometryIndex2,
        Base::Vector3d& intersect2, double& u1, double& u2)
    {
        if (geometryIndex >= mGeoList.size()) {
            return false;
        }
        gp_Pln plane(gp_Pnt(0, 0, 0), gp_Dir(0, 0, 1));

        Standard_Boolean periodic = Standard_False;
        double period = 0;
        Handle(Geom2d_Curve) primaryCurve;
        Handle(Geom_Geometry) geom = (mGeoList[geometryIndex])->handle();
        Handle(Geom_Curve) curve3d = Handle(Geom_Curve)::DownCast(geom);

        if (curve3d.IsNull()) {
            return false;
        }
        else {
            primaryCurve = GeomAPI::To2d(curve3d, plane);
            periodic = primaryCurve->IsPeriodic();
            if (periodic) {
                period = primaryCurve->Period();
            }
        }

        // create the intersector and projector functions
        Geom2dAPI_InterCurveCurve Intersector;
        Geom2dAPI_ProjectPointOnCurve Projector;

        // find the parameter of the picked point on the primary curve
        Projector.Init(gp_Pnt2d(point.x, point.y), primaryCurve);
        double pickedParam = Projector.LowerDistanceParameter();

        // find intersection points
        geometryIndex1 = -1;
        geometryIndex2 = -1;
        double param1 = -1e10, param2 = 1e10;
        gp_Pnt2d p1, p2;
        Handle(Geom2d_Curve) secondaryCurve;
        for (int id = 0; id < int(mGeoList.size()); id++) {
            // #0000624: Trim tool doesn't work with construction lines
            if (id != geometryIndex /* && !geomlist[id]->Construction*/) {
                geom = (mGeoList[id])->handle();
                curve3d = Handle(Geom_Curve)::DownCast(geom);
                if (!curve3d.IsNull()) {
                    secondaryCurve = GeomAPI::To2d(curve3d, plane);
                    // perform the curves intersection

                    std::vector<gp_Pnt2d> points;

                    // #2463 Check for endpoints of secondarycurve on primary curve
                    // If the OCCT Intersector should detect endpoint tangency when trimming, then
                    // this is just a work-around until that bug is fixed.
                    // https://www.freecad.org/tracker/view.php?id=2463
                    // https://tracker.dev.opencascade.org/view.php?id=30217
                    if (mGeoList[id]->isDerivedFrom<Part::GeomBoundedCurve>()) {

                        Part::GeomBoundedCurve* bcurve = static_cast<Part::GeomBoundedCurve*>(mGeoList[id].get());

                        points.emplace_back(bcurve->getStartPoint().x, bcurve->getStartPoint().y);
                        points.emplace_back(bcurve->getEndPoint().x, bcurve->getEndPoint().y);
                    }

                    Intersector.Init(primaryCurve, secondaryCurve, 1.0e-12);

                    for (int i = 1; i <= Intersector.NbPoints(); i++) {
                        points.push_back(Intersector.Point(i));
                    }

                    if (Intersector.NbSegments() > 0) {
                        const Geom2dInt_GInter& gInter = Intersector.Intersector();
                        for (int i = 1; i <= gInter.NbSegments(); i++) {
                            const IntRes2d_IntersectionSegment& segm = gInter.Segment(i);
                            if (segm.HasFirstPoint()) {
                                const IntRes2d_IntersectionPoint& fp = segm.FirstPoint();
                                points.push_back(fp.Value());
                            }
                            if (segm.HasLastPoint()) {
                                const IntRes2d_IntersectionPoint& fp = segm.LastPoint();
                                points.push_back(fp.Value());
                            }
                        }
                    }

                    for (auto p : points) {
                        // get the parameter of the intersection point on the primary curve
                        Projector.Init(p, primaryCurve);

                        if (Projector.NbPoints() < 1
                            || Projector.LowerDistance() > Precision::Confusion()) {
                            continue;
                        }

                        double param = Projector.LowerDistanceParameter();

                        if (periodic) {
                            // transfer param into the interval (pickedParam-period pickedParam]
                            param = param - period * ceil((param - pickedParam) / period);
                            if (param > param1) {
                                param1 = param;
                                u1 = param1;
                                p1 = p;
                                geometryIndex1 = id;
                            }
                            param -= period;  // transfer param into the interval (pickedParam
                            // pickedParam+period]
                            if (param < param2) {
                                param2 = param;
                                u2 = param2;
                                p2 = p;
                                geometryIndex2 = id;
                            }
                        }
                        else if (param < pickedParam && param > param1) {
                            param1 = param;
                            p1 = p;
                            geometryIndex1 = id;
                            u1 = param1;
                        }
                        else if (param > pickedParam && param < param2) {
                            param2 = param;
                            u2 = param2;
                            p2 = p;
                            geometryIndex2 = id;
                        }
                    }
                }
            }
        }
        if (periodic) {
            // in case both points coincide, cancel the selection of one of both
            if (fabs(param2 - param1 - period) < 1e-10) {
                if (param2 - pickedParam >= pickedParam - param1) {
                    geometryIndex2 = -1;
                }
                else {
                    geometryIndex1 = -1;
                }
            }
        }

        //if (geometryIndex1 < 0 && geometryIndex2 < 0) {
        //    return false;
        //}

        if (geometryIndex1 >= 0) {
            intersect1 = Base::Vector3d(p1.X(), p1.Y(), 0.f);
        }
        else
        {
            const auto* geoAsCurve = static_cast<Part::GeomCurve*>(mGeoList[geometryIndex].get());
            u1 = geoAsCurve->getFirstParameter();
            intersect1 = geoAsCurve->value(u1);

        }
        if (geometryIndex2 >= 0) {
            intersect2 = Base::Vector3d(p2.X(), p2.Y(), 0.f);
        }
        else
        {
            const auto* geoAsCurve = static_cast<Part::GeomCurve*>(mGeoList[geometryIndex].get());
            u2 = geoAsCurve->getLastParameter();
            intersect2 = geoAsCurve->value(u2);
        }
        return true;
    }
    void SketcherObj::deleteGeometry(int GeoId)
    {
        if (GeoId < mGeoList.size()) {
		    auto it = mGeoList.begin();
		    std::advance(it, GeoId);
		    mGeoSegment.erase((*it).get());
		    mGeoList.erase(it);
        }
    }
    void SketcherObj::deleteGeometries(const std::vector<int>& GeoIds)
    {
        if (GeoIds.size() == 0) {
            return;
        }
        const int oldSize = static_cast<int>(mGeoList.size());
        std::vector<int> deletePos(oldSize, 0);
		for (int i = 0;i < GeoIds.size();i++) {
			if (GeoIds[i] >= 0 && GeoIds[i] < oldSize) {
				deletePos[GeoIds[i]] = 1;
			}
		}
        // Map every surviving old index to its new index after removal.
        std::vector<int> newIndex(oldSize, -1);
        int nextIndex = 0;
        for (int i = 0; i < oldSize; ++i) {
            if (!deletePos[i]) {
                newIndex[i] = nextIndex++;
            }
        }

		auto it = mGeoList.begin();
		int index = 0;
		while (it != mGeoList.end()) {
			if (deletePos[index] == 1) {
				mGeoSegment.erase((*it).get());
				it = mGeoList.erase(it);
                if (index >= oldSize) {
                    break;
                }
			}
			else {
				it++;
			}
			index++;
		}

        // FreeCAD deletes every constraint that references a removed geometry
        // and shifts the GeoIds of all constraints after the deletion point.
        auto remapGeoId = [&](int& geoId) -> bool {
            if (geoId >= 0 && geoId < oldSize) {
                if (deletePos[geoId]) {
                    return false;  // constraint refers to a deleted element
                }
                geoId = newIndex[geoId];
            }
            return true;
        };

        std::vector<Sketcher::Constraint*> keptConstraints;
        keptConstraints.reserve(mConstraintList.size());
        for (Sketcher::Constraint* c : mConstraintList) {
            bool keep = remapGeoId(c->First);
            keep = keep && remapGeoId(c->Second);
            keep = keep && remapGeoId(c->Third);
            if (keep) {
                keptConstraints.push_back(c);
            }
            else {
                delete c;
            }
        }
        mConstraintList = std::move(keptConstraints);
        // Deleted constraints invalidate the label overlay bookkeeping, which
        // is keyed by constraint pointer.
        m_labelManualOffsetPx.clear();
        m_labelManualParam.clear();
        m_labelHover = -1;
        m_labelDrag = -1;
    }
    void SketcherObj::replaceGeometry(int oldGeoId, std::unique_ptr<Part::Geometry>& newGeo)
    {
        if (oldGeoId < mGeoList.size()) {
            mGeoSegment.erase(mGeoList[oldGeoId].get());
			mGeoList[oldGeoId] = std::move((newGeo));
            mGeoSegment[mGeoList[oldGeoId].get()] = getCurveSegment(mGeoList[oldGeoId].get());
        }
    }
    void SketcherObj::replaceGeometries(const std::vector<int>& oldGeoIds,  std::vector<std::unique_ptr<Part::Geometry>>& newGeos)
    {
        int i = 0;
        for (;i < oldGeoIds.size()&&i< newGeos.size();i++) {
			int oldGeoId = oldGeoIds[i];
			if (oldGeoId < mGeoList.size()) {
                replaceGeometry(oldGeoId,newGeos[i]);
			}
        }
        for (;i < newGeos.size();i++) {
            addGeometry(newGeos[i]);
        }
    }
    bool SketcherObj::isClosedCurve(const Part::Geometry* geo)
    {
        return (geo->is<Part::GeomCircle>()
            || geo->is<Part::GeomEllipse>()
            || (geo->is<Part::GeomBSplineCurve>()
                && static_cast<const Part::GeomBSplineCurve*>(geo)->isPeriodic()));
    }
    bool SketcherObj::trim(int GeoId, double u0, double u1,const Base::Vector3d& point0, const Base::Vector3d& point1)
    {
        const auto* geoAsCurve = static_cast<Part::GeomCurve*>(mGeoList[GeoId].get());
        std::vector<std::pair<double, double>> paramsOfNewGeos;
        paramsOfNewGeos.reserve(2);
        double firstParam = geoAsCurve->getFirstParameter();
        double lastParam = geoAsCurve->getLastParameter();
        double cut0Param{ u0 }, cut1Param{ u1 };
		bool isClosed = isClosedCurve(geoAsCurve);
        int numUndefs=0;
        bool cut0IsUndef = false;
        bool cut1IsUndef = false;
        if (!isClosed) {
			if (areParamsWithinApproximation(cut0Param, firstParam)) {
                numUndefs++;
				cut0IsUndef = true;
			}
			if (areParamsWithinApproximation(cut1Param, lastParam)) {
                numUndefs++;
				cut1IsUndef = true;
			}
        }
        if (numUndefs == 0 && arePointsWithinPrecision(point0,point1)) {
            // If both points are detected and are coincident, deletion is the only option.
            paramsOfNewGeos.clear();
        }
        else
        {
            paramsOfNewGeos.assign(2 - numUndefs, { firstParam, lastParam });
            if (isClosed) {
                paramsOfNewGeos.pop_back();
            }
            if (!cut0IsUndef) {
                paramsOfNewGeos.front().second = cut0Param;
            }
            if (!cut1IsUndef) {
                paramsOfNewGeos.back().first = cut1Param;
            }
        }

        std::vector<int> newIds;
        std::vector<std::unique_ptr<Part::Geometry>> newGeos;
        switch (paramsOfNewGeos.size()) {
            case 0: {
                {
					deleteGeometry(GeoId);
                }
                return true;
            }
            case 1: {
                newIds.push_back(GeoId);
                break;
            }
            case 2: {
                newIds.push_back(GeoId);
                newIds.push_back(mGeoList.size());
                break;
            }
            default: {
                return false;
            }
        }
        for (auto& [param1, param2] : paramsOfNewGeos) {
            Part::Geometry* newGeo = (geoAsCurve)->createArc(param1, param2);
            assert(newGeo);
			std::unique_ptr<Part::Geometry> newGeoPtr(newGeo);
            newGeos.push_back(std::move(newGeoPtr));
        }
        replaceGeometries({GeoId},newGeos);
        return true;
    }
    int SketcherObj::addSymmetric(const std::vector<int>& geoIdList, int refGeoId)
    {

        std::map<int, int> geoIdMap;
        std::map<int, bool> isStartEndInverted;
        std::vector<Part::Geometry*> symgeos= getSymmetric(geoIdList, geoIdMap, isStartEndInverted, refGeoId);
        addGeometry(symgeos);
        return geoIdList.size() - 1;
    }
    std::vector<Part::Geometry*> SketcherObj::getSymmetric(const std::vector<int>& geoIdList, std::map<int, int>& geoIdMap, std::map<int, bool>& isStartEndInverted, int refGeoId)
    {
        std::vector<Part::Geometry*> symmetricVals;
        
        int cgeoid = getHighestCurveIndex() + 1;

        const Part::Geometry* georef = getGeometry(refGeoId);
        if (!georef->is<Part::GeomLineSegment>()) {
            return {};
        }

        auto* refGeoLine = static_cast<const Part::GeomLineSegment*>(georef);
        // line
        Base::Vector3d refstart = refGeoLine->getStartPoint();
        Base::Vector3d vectline = refGeoLine->getEndPoint() - refstart;

        for (auto geoId : geoIdList) {
            const Part::Geometry* geo = getGeometry(geoId);
            Part::Geometry* geosym;

            geosym = geo->copy();

            // Handle Geometry
            if (geosym->is<Part::GeomLineSegment>()) {
                auto* geosymline = static_cast<Part::GeomLineSegment*>(geosym);
                Base::Vector3d sp = geosymline->getStartPoint();
                Base::Vector3d ep = geosymline->getEndPoint();

                geosymline->setPoints(
                    sp + 2.0 * (sp.Perpendicular(refGeoLine->getStartPoint(), vectline) - sp),
                    ep + 2.0 * (ep.Perpendicular(refGeoLine->getStartPoint(), vectline) - ep)
                );
                isStartEndInverted.insert(std::make_pair(geoId, false));
            }
            else if (geosym->is<Part::GeomCircle>()) {
                auto* geosymcircle = static_cast<Part::GeomCircle*>(geosym);
                Base::Vector3d cp = geosymcircle->getCenter();

                geosymcircle->setCenter(
                    cp + 2.0 * (cp.Perpendicular(refGeoLine->getStartPoint(), vectline) - cp)
                );
                isStartEndInverted.insert(std::make_pair(geoId, false));
            }
            else if (geosym->is<Part::GeomArcOfCircle>()) {
                auto* geoaoc = static_cast<Part::GeomArcOfCircle*>(geosym);
                Base::Vector3d sp = geoaoc->getStartPoint(true);
                Base::Vector3d ep = geoaoc->getEndPoint(true);
                Base::Vector3d cp = geoaoc->getCenter();

                Base::Vector3d ssp = sp
                    + 2.0 * (sp.Perpendicular(refGeoLine->getStartPoint(), vectline) - sp);
                Base::Vector3d sep = ep
                    + 2.0 * (ep.Perpendicular(refGeoLine->getStartPoint(), vectline) - ep);
                Base::Vector3d scp = cp
                    + 2.0 * (cp.Perpendicular(refGeoLine->getStartPoint(), vectline) - cp);

                double theta1 = Base::fmod(atan2(sep.y - scp.y, sep.x - scp.x), 2.f * 3.1415926535);
                double theta2 = Base::fmod(atan2(ssp.y - scp.y, ssp.x - scp.x), 2.f * 3.1415926535);

                geoaoc->setCenter(scp);
                geoaoc->setRange(theta1, theta2, true);
                isStartEndInverted.insert(std::make_pair(geoId, true));
            }
            else if (geosym->is<Part::GeomEllipse>()) {
                auto* geosymellipse = static_cast<Part::GeomEllipse*>(geosym);
                Base::Vector3d cp = geosymellipse->getCenter();

                Base::Vector3d majdir = geosymellipse->getMajorAxisDir();
                double majord = geosymellipse->getMajorRadius();
                double minord = geosymellipse->getMinorRadius();
                double df = sqrt(majord * majord - minord * minord);
                Base::Vector3d f1 = cp + df * majdir;

                Base::Vector3d sf1 = f1
                    + 2.0 * (f1.Perpendicular(refGeoLine->getStartPoint(), vectline) - f1);
                Base::Vector3d scp = cp
                    + 2.0 * (cp.Perpendicular(refGeoLine->getStartPoint(), vectline) - cp);

                geosymellipse->setMajorAxisDir(sf1 - scp);

                geosymellipse->setCenter(scp);
                isStartEndInverted.insert(std::make_pair(geoId, false));
            }
            else if (geosym->is<Part::GeomArcOfEllipse>()) {
                auto* geosymaoe = static_cast<Part::GeomArcOfEllipse*>(geosym);
                Base::Vector3d cp = geosymaoe->getCenter();

                Base::Vector3d majdir = geosymaoe->getMajorAxisDir();
                double majord = geosymaoe->getMajorRadius();
                double minord = geosymaoe->getMinorRadius();
                double df = sqrt(majord * majord - minord * minord);
                Base::Vector3d f1 = cp + df * majdir;

                Base::Vector3d sf1 = f1
                    + 2.0 * (f1.Perpendicular(refGeoLine->getStartPoint(), vectline) - f1);
                Base::Vector3d scp = cp
                    + 2.0 * (cp.Perpendicular(refGeoLine->getStartPoint(), vectline) - cp);

                geosymaoe->setMajorAxisDir(sf1 - scp);

                geosymaoe->setCenter(scp);

                double theta1, theta2;
                geosymaoe->getRange(theta1, theta2, true);
                theta1 = 2.0 * 3.1415926535 - theta1;
                theta2 = 2.0 * 3.1415926535 - theta2;
                std::swap(theta1, theta2);
                if (theta1 < 0) {
                    theta1 += 2.0 * 3.1415926535;
                    theta2 += 2.0 * 3.1415926535;
                }

                geosymaoe->setRange(theta1, theta2, true);
                isStartEndInverted.insert(std::make_pair(geoId, true));
            }
            else if (geosym->is<Part::GeomArcOfHyperbola>()) {
                auto* geosymaoe = static_cast<Part::GeomArcOfHyperbola*>(geosym);
                Base::Vector3d cp = geosymaoe->getCenter();

                Base::Vector3d majdir = geosymaoe->getMajorAxisDir();
                double majord = geosymaoe->getMajorRadius();
                double minord = geosymaoe->getMinorRadius();
                double df = sqrt(majord * majord + minord * minord);
                Base::Vector3d f1 = cp + df * majdir;

                Base::Vector3d sf1 = f1
                    + 2.0 * (f1.Perpendicular(refGeoLine->getStartPoint(), vectline) - f1);
                Base::Vector3d scp = cp
                    + 2.0 * (cp.Perpendicular(refGeoLine->getStartPoint(), vectline) - cp);

                geosymaoe->setMajorAxisDir(sf1 - scp);

                geosymaoe->setCenter(scp);

                double theta1, theta2;
                geosymaoe->getRange(theta1, theta2, true);
                theta1 = -theta1;
                theta2 = -theta2;
                std::swap(theta1, theta2);

                geosymaoe->setRange(theta1, theta2, true);
                isStartEndInverted.insert(std::make_pair(geoId, true));
            }
            else if (geosym->is<Part::GeomArcOfParabola>()) {
                auto* geosymaoe = static_cast<Part::GeomArcOfParabola*>(geosym);
                Base::Vector3d cp = geosymaoe->getCenter();

                Base::Vector3d f1 = geosymaoe->getFocus();

                Base::Vector3d sf1 = f1
                    + 2.0 * (f1.Perpendicular(refGeoLine->getStartPoint(), vectline) - f1);
                Base::Vector3d scp = cp
                    + 2.0 * (cp.Perpendicular(refGeoLine->getStartPoint(), vectline) - cp);

                geosymaoe->setXAxisDir(sf1 - scp);
                geosymaoe->setCenter(scp);

                double theta1, theta2;
                geosymaoe->getRange(theta1, theta2, true);
                theta1 = -theta1;
                theta2 = -theta2;
                std::swap(theta1, theta2);

                geosymaoe->setRange(theta1, theta2, true);
                isStartEndInverted.insert(std::make_pair(geoId, true));
            }
            else if (geosym->is<Part::GeomBSplineCurve>()) {
                auto* geosymbsp = static_cast<Part::GeomBSplineCurve*>(geosym);

                std::vector<Base::Vector3d> poles = geosymbsp->getPoles();

                for (auto& pole : poles) {
                    pole = pole
                        + 2.0 * (pole.Perpendicular(refGeoLine->getStartPoint(), vectline) - pole);
                }

                geosymbsp->setPoles(poles);

                isStartEndInverted.insert(std::make_pair(geoId, false));
            }
            else if (geosym->is<Part::GeomPoint>()) {
                auto* geosympoint = static_cast<Part::GeomPoint*>(geosym);
                Base::Vector3d cp = geosympoint->getPoint();

                geosympoint->setPoint(
                    cp + 2.0 * (cp.Perpendicular(refGeoLine->getStartPoint(), vectline) - cp)
                );
                isStartEndInverted.insert(std::make_pair(geoId, false));
            }
            else {
                CORE_ERROR("Unsupported Geometry!! Just copying it.\n");
                isStartEndInverted.insert(std::make_pair(geoId, false));
            }
            symmetricVals.push_back(geosym);
            geoIdMap.insert(std::make_pair(geoId, cgeoid));
            cgeoid++;
        }
        return symmetricVals;
    }
    Part::TopoShape SketcherObj::toShape() const
    {
        Part::TopoShape result;
        std::list<TopoDS_Edge> edge_list;
        std::list<TopoDS_Wire> wires;
		for (const auto& geo : mGeoList) {
			auto shape = geo->toShape();
			if (shape.ShapeType() == TopAbs_EDGE) {
				edge_list.push_back(TopoDS::Edge(shape));
			}
		}
        // Hint: Use ShapeAnalysis_FreeBounds::ConnectEdgesToWires() as an alternative
        // sort them together to wires
        while (!edge_list.empty()) {
            BRepBuilderAPI_MakeWire mkWire;
            // add and erase first edge
            mkWire.Add(edge_list.front());
            edge_list.pop_front();
            TopoDS_Wire new_wire = mkWire.Wire();  // current new wire
            // try to connect each edge to the wire, the wire is complete if no more edges are
            // connectible
            bool found = false;
            do {
                found = false;
                for (auto pE = edge_list.begin(); pE != edge_list.end(); ++pE) {
                    mkWire.Add(*pE);
                    if (mkWire.Error() != BRepBuilderAPI_DisconnectedWire) {
                        // edge added ==> remove it from list
                        found = true;
                        edge_list.erase(pE);
                        new_wire = mkWire.Wire();
                        break;
                    }
                }
            } while (found);

            // Fix any topological issues of the wire
            ShapeFix_Wire aFix;
            aFix.SetPrecision(Precision::Confusion());
            aFix.Load(new_wire);
            aFix.FixReorder();
            aFix.FixConnected();
            aFix.FixClosed();
            wires.push_back(aFix.Wire());
        }

        if (wires.size() == 1 ) {
            result = *wires.begin();
        }
        else if (wires.size() > 1 ) {
            BRep_Builder builder;
            TopoDS_Compound comp;
            builder.MakeCompound(comp);
            for (auto& wire : wires) {
                builder.Add(comp, wire);
            }
            result.setShape(comp);
        }
        result.setTransform(planeTransform);
        return result;
    }
    Base::Matrix4D SketcherObj::getplaneTransform()
    {
        return planeTransform;
    }
    int SketcherObj::addConstraint(const Sketcher::Constraint* constraint)
    {
        auto constraint_ptr = std::unique_ptr<Sketcher::Constraint>(constraint->clone());
        return addConstraint(std::move(constraint_ptr));
    }
    int  SketcherObj::addConstraint(std::unique_ptr<Sketcher::Constraint> constraint)
    {
        if (!constraint) {
            return -1;
        }

        // Basic index validation: elements used by a constraint must exist.
        auto isValidGeoId = [this](int geoId) {
            return geoId < 0 || (geoId < static_cast<int>(mGeoList.size()));
        };
        if (!isValidGeoId(constraint->First) || !isValidGeoId(constraint->Second)
            || !isValidGeoId(constraint->Third)) {
            return -2;
        }

        for (int i = 0; i < mConstraintList.size(); i++) {
            if (
                mConstraintList[i]->Type == constraint->Type &&
                mConstraintList[i]->First == constraint->First &&
                mConstraintList[i]->FirstPos == constraint->FirstPos &&
                mConstraintList[i]->Second == constraint->Second &&
                mConstraintList[i]->SecondPos == constraint->SecondPos &&
                mConstraintList[i]->Third == constraint->Third &&
                mConstraintList[i]->ThirdPos == constraint->ThirdPos
                )
            {
                return -1;
            }
        }
        Sketcher::Constraint* constNew = constraint.release();
        mConstraintList.push_back(constNew);
        return mConstraintList.size()-1;
    }
    const Sketcher::Constraint* SketcherObj::getConstraint(int index) const
    {
        if (index < 0 || index >= static_cast<int>(mConstraintList.size())) {
            return nullptr;
        }
        return mConstraintList[index];
    }
    int SketcherObj::findConstraint(const Sketcher::Constraint* pattern) const
    {
        if (!pattern) {
            return -1;
        }
        for (int i = 0; i < static_cast<int>(mConstraintList.size()); ++i) {
            const Sketcher::Constraint* c = mConstraintList[i];
            if (c->Type == pattern->Type && c->First == pattern->First
                && c->FirstPos == pattern->FirstPos && c->Second == pattern->Second
                && c->SecondPos == pattern->SecondPos && c->Third == pattern->Third
                && c->ThirdPos == pattern->ThirdPos) {
                return i;
            }
        }
        return -1;
    }
    int SketcherObj::setDatum(int constrId, double datum)
    {
        if (constrId < 0 || constrId >= static_cast<int>(mConstraintList.size())) {
            return -1;
        }

        Sketcher::Constraint* c = mConstraintList[constrId];
        if (!c->isDimensional() && c->Type != Sketcher::ConstraintType::Tangent
            && c->Type != Sketcher::ConstraintType::Perpendicular) {
            return -1;
        }

        const double oldValue = c->getValue();
        c->setValue(datum);
        const int err = solve();
        if (err != 0) {
            c->setValue(oldValue);  // keep the sketch consistent with the old datum
        }
        return err;
    }
    void SketcherObj::addConstraint(Sketcher::ConstraintType constrType, int firstGeoId, Sketcher::PointPos firstPos, int secondGeoId, Sketcher::PointPos secondPos, int thirdGeoId, Sketcher::PointPos thirdPos)
    {
        auto newConstr = createConstraint(
            constrType, firstGeoId, firstPos, secondGeoId, secondPos, thirdGeoId, thirdPos);

        this->addConstraint(std::move(newConstr));
    }
    std::unique_ptr<Sketcher::Constraint> SketcherObj::createConstraint(Sketcher::ConstraintType constrType, int firstGeoId, Sketcher::PointPos firstPos, int secondGeoId, Sketcher::PointPos secondPos, int thirdGeoId, Sketcher::PointPos thirdPos)
    {
        auto newConstr = std::make_unique<Sketcher::Constraint>();

        newConstr->Type = constrType;
        newConstr->First = firstGeoId;
        newConstr->FirstPos = firstPos;
        newConstr->Second = secondGeoId;
        newConstr->SecondPos = secondPos;
        newConstr->Third = thirdGeoId;
        newConstr->ThirdPos = thirdPos;
        return newConstr;
    }
    void SketcherObj::retrieveSolverDiagnostics()
    {
        lastHasConflict = solvedSketch.hasConflicts();
        lastHasRedundancies = solvedSketch.hasRedundancies();
        lastHasPartialRedundancies = solvedSketch.hasPartialRedundancies();
        lastHasMalformedConstraints = solvedSketch.hasMalformedConstraints();
        lastConflicting = solvedSketch.getConflicting();
        lastRedundant = solvedSketch.getRedundant();
        lastPartiallyRedundant = solvedSketch.getPartiallyRedundant();
        lastMalformedConstraints = solvedSketch.getMalformedConstraints();
    }
    void SketcherObj::updateGeoSegment(int id)
    {
        if (id < mGeoList.size()) {
			mGeoSegment[mGeoList[id].get()] = getCurveSegment(mGeoList[id].get());
        }
    }
    void SketcherObj::pickGeo()
    {
        onSketchPosMove = getMouseHitSketchPlanePoint();
        preSelectGeoId = testSelect(onSketchPosMove);
        std::set<int>avoidList;
        avoidList.insert(preSelectGeoId.GeoId);
        snapPoint(onSketchPosMove, avoidList);
    }
    void SketcherObj::clearSelect() {
        selectIds.clear();
    }
    void SketcherObj::moveGeo(SelectGeoId Id, float dx, float dy)
    {
        if (Id.GeoId < mGeoList.size()) {
            int geoId = Id.GeoId;
            Part::Geometry* geo = mGeoList[geoId].get();
            bool isStart = Id.pointPos == PointPos::start;
            bool isEnd = Id.pointPos == PointPos::end;
            bool isCenter= Id.pointPos == PointPos::mid;
            bool isNone = Id.pointPos == PointPos::none;
            Base::Vector3d delta(dx, dy, 0);
            Base::Vector3d mousePos = Base::Vector3d(onSketchPosMove.x,onSketchPosMove.y,0.0);

            { 
            if (geo->isDerivedFrom<Part::GeomCurve>()) {
                if (geo->is<Part::GeomArcOfCircle>()) {
                    Part::GeomArcOfCircle* curve = static_cast<Part::GeomArcOfCircle*>(geo);
                    if (isNone) {
                        curve->setRadius((mousePos - curve->getCenter()).Length());
                    }
                    else if (isCenter) {
                        curve->setCenter(mousePos);
                    }
                    else
                    {
                        double u, v;
                        curve->getRange(u,v,false);
                        Base::Vector3d deltaV=mousePos - curve->getCenter();
                        Base::Vector3d xAxis = Base::Vector3d(1, 0, 0);
                        bool isNegative=xAxis.Cross(deltaV).z<0;
                        double angle = (deltaV).GetAngle(Base::Vector3d(1, 0, 0));
                        if (isNegative) {
                            angle = -angle;
                        }
                        if (isStart) {
                            curve->setRange(angle,v,false);
                        }
                        else if (isEnd) {
                            curve->setRange(u, angle, false);
                        }
                    }
                }
                else if (geo->is<Part::GeomLineSegment>()) {
                    Part::GeomLineSegment* lineSeg = static_cast<Part::GeomLineSegment*>(geo);
                    if (isStart) {
                        lineSeg->setPoints(mousePos,lineSeg->getEndPoint());
                    }
                    else if(isEnd) {
                        lineSeg->setPoints(lineSeg->getStartPoint() , mousePos);
                    }
                    else
                    {
                        geo->translate(delta);
                    }
                }
                else if (geo->is<Part::GeomArcOfConic>()) {
                    Part::GeomArcOfConic* curve = static_cast<Part::GeomArcOfConic*>(geo);
                       
                }
                else if (geo->is<Part::GeomCircle>()) {
                    Part::GeomCircle* curve = static_cast<Part::GeomCircle*>(geo);
                    if (isNone) {
                        curve->setRadius((mousePos-curve->getCenter()).Length());
                    }
                    else
                    {
                        curve->setCenter(mousePos);
                    }
                }
                else if (geo->is<Part::GeomBSplineCurve>()) {
                    Part::GeomBSplineCurve* curve = static_cast<Part::GeomBSplineCurve*>(geo);
                    geo->translate(delta);
                }
            }
            }
            updateGeoSegment(geoId);
        }
    }
    void SketcherObj::addSelect(SelectGeoId geoId)
    {
        bool existflag = false;
        for (int i = 0; i < selectIds.size(); i++) {
            if (selectIds[i].GeoId == geoId.GeoId && selectIds[i].pointPos == geoId.pointPos) {
                existflag = true;
                break;
            }
        }
        if (!existflag) {
            selectIds.push_back(geoId);
        }
    }
    Base::Matrix4D SketcherObj::updateTransform() const
    {
        Base::Matrix4D ret;
        ret = Base::Matrix4D(
            mPlane.xAxis.x, mPlane.yAxis.x, mPlane.normal.x, mPlane.origin.x,
            mPlane.xAxis.y, mPlane.yAxis.y, mPlane.normal.y, mPlane.origin.y,
            mPlane.xAxis.z, mPlane.yAxis.z, mPlane.normal.z, mPlane.origin.z,
            0.0, 0.0, 0.0, 1.0
        );
        return ret;
    }
    Base::Vector2d SketcherObj::getMouseHitSketchPlanePoint()
    {
        auto ray = m_sceneView->GetMouseRay();
        Maths::FVector3 out;
        Base::Vector2d onSketchPos;
        ray.hitPlane(Maths::FVector3(mPlane.normal.x, mPlane.normal.y, mPlane.normal.z), mPlane.normal.Dot(mPlane.origin), out);
        Base::Vector3d hitPos{out.x,out.y,out.z};
        double x= (hitPos - mPlane.origin).Dot(mPlane.xAxis);
        double y=(hitPos - mPlane.origin).Dot(mPlane.yAxis);
        onSketchPos = Base::Vector2d(int(x * 100) / 100.0, int(y * 100) / 100.0);
        return onSketchPos;
    }
    SketcherObj::CurveSegment SketcherObj::getCurveSegment(Part::Geometry* geo) 
    {
		CurveSegment seg;
		CurveConvert::toVector2D(geo, 50, seg.point, seg.params);
        if (geo->isDerivedFrom<Part::GeomCurve>()) {
            if (geo->is<Part::GeomArcOfCircle>()) {
                Part::GeomArcOfCircle* curve = static_cast<Part::GeomArcOfCircle*>(geo);
                seg.sepoints.push_back({ curve->getStartPoint(),PointPos::start });
                seg.sepoints.push_back({ curve->getEndPoint() ,PointPos::end});
                seg.sepoints.push_back({ curve->getCenter() ,PointPos::mid});
            }
            else if (geo->is<Part::GeomLineSegment>()) {
                Part::GeomLineSegment* lineSeg = static_cast<Part::GeomLineSegment*>(geo);
                seg.sepoints.push_back({ lineSeg->getStartPoint(),PointPos::start });
                seg.sepoints.push_back({ lineSeg->getEndPoint(),PointPos::end });
            }
            else if (geo->is<Part::GeomArcOfConic>()) {
                Part::GeomArcOfConic* curve = static_cast<Part::GeomArcOfConic*>(geo);
                seg.sepoints.push_back({ curve->getStartPoint(),PointPos::start });
                seg.sepoints.push_back({ curve->getEndPoint() ,PointPos::end });
                seg.sepoints.push_back({ curve->getCenter() ,PointPos::mid });
            }
            else if (geo->is<Part::GeomCircle>()) {
                Part::GeomCircle* curve = static_cast<Part::GeomCircle*>(geo);
                seg.sepoints.push_back({ curve->getCenter() ,PointPos::mid });
            }
            else if (geo->is<Part::GeomBSplineCurve>()) {
                Part::GeomBSplineCurve* curve = static_cast<Part::GeomBSplineCurve*>(geo);
                std::vector<Base::Vector3d>poles= curve->getPoles();
                for (int i = 0; i < poles.size(); i++) {
                    seg.sepoints.push_back({ poles[i],PointPos::mid });
                }
            }
        }
        else if (geo->is<Part::GeomPoint>()) {
            Base::Vector3d pos = static_cast<Part::GeomPoint*>(geo)->getPoint();
            seg.sepoints.push_back({ pos ,PointPos::mid });
        }
        return seg;
    }
    // ------------------------------------------------------------------
    // Dimension label overlay (P0)
    // ------------------------------------------------------------------
    static void projectPointOnSegment2d(
        const Base::Vector2d& point,
        const Base::Vector2d& a,
        const Base::Vector2d& b,
        Base::Vector2d& proj
    )
    {
        Base::Vector2d ab(b.x - a.x, b.y - a.y);
        const double len2 = ab.x * ab.x + ab.y * ab.y;
        if (len2 < 1.0e-12) {
            proj = a;
            return;
        }
        double t = ((point.x - a.x) * ab.x + (point.y - a.y) * ab.y) / len2;
        t = std::max(0.0, std::min(1.0, t));
        proj.x = a.x + t * ab.x;
        proj.y = a.y + t * ab.y;
    }
    static bool intersectLines2d(
        const Base::Vector2d& p1,
        const Base::Vector2d& p2,
        const Base::Vector2d& p3,
        const Base::Vector2d& p4,
        Base::Vector2d& out
    )
    {
        const double dx1 = p2.x - p1.x;
        const double dy1 = p2.y - p1.y;
        const double dx2 = p4.x - p3.x;
        const double dy2 = p4.y - p3.y;
        const double det = dx1 * dy2 - dy1 * dx2;
        if (std::abs(det) < 1.0e-12) {
            return false;
        }
        const double t = ((p3.x - p1.x) * dy2 - (p3.y - p1.y) * dx2) / det;
        const double u = ((p3.x - p1.x) * dy1 - (p3.y - p1.y) * dx1) / det;
        // Accept slightly extended segments so shared-corner angles work even
        // when the polyline endpoints have tiny numeric gaps.
        if (t < -0.25 || t > 1.25 || u < -0.25 || u > 1.25) {
            return false;
        }
        out.x = p1.x + t * dx1;
        out.y = p1.y + t * dy1;
        return true;
    }
    void SketcherObj::updateConstraintLabelInteraction()
    {
        // While geometry itself is being dragged the label overlay must stay
        // out of the way; a label drag, however, is always continued here.
        if (isHaveActiveHandler || !isInEdit
            || (selectState == OperationGeo && m_labelDrag < 0)) {
            m_labelHover = -1;
            m_labelDrag = -1;
            return;
        }
        auto [mx, my] = m_sceneView->getInutState().GetMousePosition();
        if (m_labelDrag >= 0) {
            const Sketcher::Constraint* c = getConstraint(m_labelDrag);
            if (!c) {
                m_labelDrag = -1;
                return;
            }
            const bool straightDim = c->Type == Sketcher::ConstraintType::Distance
                || c->Type == Sketcher::ConstraintType::DistanceX
                || c->Type == Sketcher::ConstraintType::DistanceY;
            if (straightDim) {
                // Project the mouse onto the dimension shaft: the caption may
                // only slide along the segment, it never leaves the line.
                float trackAx = 0.0f, trackAy = 0.0f;
                float trackBx = 0.0f, trackBy = 0.0f;
                float gapX = 0.0f, gapY = 0.0f;
                if (computeStraightLabelTrack(
                        c, trackAx, trackAy, trackBx, trackBy, gapX, gapY
                    )) {
                    const float dirX = trackBx - trackAx;
                    const float dirY = trackBy - trackAy;
                    const float len2 = dirX * dirX + dirY * dirY;
                    if (len2 > 1.0f) {
                        float t = ((static_cast<float>(mx) - trackAx) * dirX
                            + (static_cast<float>(my) - trackAy) * dirY)
                            / len2;
                        t = std::max(0.0f, std::min(1.0f, t));
                        m_labelManualParam[c] = t;
                    }
                    else {
                        m_labelManualParam[c] = 0.5;
                    }
                }
            }
            else if (c->Type == Sketcher::ConstraintType::Angle) {
                // The caption may only slide along the annotation arc:
                // convert the mouse to an angle around the arc centre and
                // store the position as a 0..1 parameter of the sweep.
                float cx = 0.0f, cy = 0.0f;
                float arcRadius = 0.0f, startDeg = 0.0f, sweepDeg = 0.0f;
                if (computeAngleLabelTrack(c, cx, cy, arcRadius, startDeg, sweepDeg)) {
                    const float mouseDeg = std::atan2(
                        static_cast<float>(my) - cy,
                        static_cast<float>(mx) - cx
                    ) * 180.0f / 3.14159265358979f;
                    float rel = mouseDeg - startDeg;
                    float t = 0.5f;
                    if (sweepDeg > 0.0f) {
                        if (rel < 0.0f) {
                            rel += 360.0f;
                        }
                        t = rel / sweepDeg;
                    }
                    else if (sweepDeg < 0.0f) {
                        if (rel > 0.0f) {
                            rel -= 360.0f;
                        }
                        t = rel / sweepDeg;
                    }
                    t = std::max(0.0f, std::min(1.0f, t));
                    m_labelManualParam[c] = t;
                }
            }
            else {
                Base::Vector2d offset = m_labelDragOffsetPx;
                offset.x += mx - m_labelDragPressPx.x;
                offset.y += my - m_labelDragPressPx.y;
                m_labelManualOffsetPx[c] = offset;
            }
            m_labelHover = m_labelDrag;
            return;
        }
        m_labelHover = pickConstraintLabelAt(static_cast<float>(mx), static_cast<float>(my));
    }
    bool SketcherObj::getGeometryPointSketch(int geoId, PointPos pos, Base::Vector2d& out) const
    {
        if (geoId == Sketcher::GeoEnum::RtPnt
            && (pos == PointPos::start || pos == PointPos::mid)) {
            out.x = 0.0;
            out.y = 0.0;
            return true;
        }
        const Part::Geometry* geo = getGeometry(geoId);
        if (!geo) {
            return false;
        }
        if (geo->is<Part::GeomPoint>()) {
            const Base::Vector3d p = static_cast<const Part::GeomPoint*>(geo)->getPoint();
            out.x = p.x;
            out.y = p.y;
            return true;
        }
        auto it = mGeoSegment.find(const_cast<Part::Geometry*>(geo));
        if (it == mGeoSegment.end()) {
            return false;
        }
        for (const auto& sp : it->second.sepoints) {
            if (sp.pointPos == pos) {
                out.x = sp.coord.x;
                out.y = sp.coord.y;
                return true;
            }
        }
        return false;
    }
    bool SketcherObj::getGeometryCenterSketch(int geoId, Base::Vector2d& out) const
    {
        if (geoId == Sketcher::GeoEnum::RtPnt) {
            out.x = 0.0;
            out.y = 0.0;
            return true;
        }
        if (geoId < 0) {
            return false;  // axes / external references are not drawn here
        }
        const Part::Geometry* geo = getGeometry(geoId);
        if (!geo) {
            return false;
        }
        Base::Vector3d center;
        if (geo->isDerivedFrom<Part::GeomConic>()) {
            center = static_cast<const Part::GeomConic*>(geo)->getCenter();
        }
        else if (geo->is<Part::GeomArcOfConic>()) {
            center = static_cast<const Part::GeomArcOfConic*>(geo)->getCenter();
        }
        else if (geo->is<Part::GeomPoint>()) {
            center = static_cast<const Part::GeomPoint*>(geo)->getPoint();
        }
        else if (geo->isDerivedFrom<Part::GeomBoundedCurve>()) {
            const auto* bounded = static_cast<const Part::GeomBoundedCurve*>(geo);
            center = (bounded->getStartPoint() + bounded->getEndPoint()) * 0.5;
        }
        else if (geo->isDerivedFrom<Part::GeomCurve>()) {
            const auto it = mGeoSegment.find(const_cast<Part::Geometry*>(geo));
            if (it != mGeoSegment.end() && !it->second.point.empty()) {
                center = it->second.point[it->second.point.size() / 2];
            }
            else {
                return false;
            }
        }
        else {
            return false;
        }
        out.x = center.x;
        out.y = center.y;
        return true;
    }
    bool SketcherObj::getConstraintMeasureEndpoints(
        const Sketcher::Constraint* constraint,
        Base::Vector2d& a,
        Base::Vector2d& b
    ) const
    {
        if (!constraint) {
            return false;
        }
        switch (constraint->Type) {
        case Sketcher::ConstraintType::DistanceX:
        case Sketcher::ConstraintType::DistanceY: {
            if (constraint->Second != Sketcher::GeoEnum::GeoUndef) {
                return getGeometryPointSketch(constraint->First, constraint->FirstPos, a)
                    && getGeometryPointSketch(constraint->Second, constraint->SecondPos, b);
            }
            if (constraint->FirstPos == PointPos::none) {
                return getGeometryPointSketch(constraint->First, PointPos::start, a)
                    && getGeometryPointSketch(constraint->First, PointPos::end, b);
            }
            return false;  // coordinate of one point: no shaft to slide on
        }
        case Sketcher::ConstraintType::Distance: {
            if (constraint->Second == Sketcher::GeoEnum::GeoUndef) {
                return getGeometryPointSketch(constraint->First, PointPos::start, a)
                    && getGeometryPointSketch(constraint->First, PointPos::end, b);
            }
            if (constraint->FirstPos != PointPos::none
                && constraint->SecondPos == PointPos::none) {
                Base::Vector2d la, lb;
                if (getGeometryPointSketch(constraint->First, constraint->FirstPos, a)
                    && getGeometryPointSketch(constraint->Second, PointPos::start, la)
                    && getGeometryPointSketch(constraint->Second, PointPos::end, lb)) {
                    projectPointOnSegment2d(a, la, lb, b);
                    return true;
                }
                return false;
            }
            if (constraint->FirstPos != PointPos::none
                && constraint->SecondPos != PointPos::none) {
                return getGeometryPointSketch(constraint->First, constraint->FirstPos, a)
                    && getGeometryPointSketch(constraint->Second, constraint->SecondPos, b);
            }
            return getGeometryCenterSketch(constraint->First, a)
                && getGeometryCenterSketch(constraint->Second, b);
        }
        default:
            return false;
        }
    }
    bool SketcherObj::computeStraightLabelTrack(
        const Sketcher::Constraint* constraint,
        float& trackAx,
        float& trackAy,
        float& trackBx,
        float& trackBy,
        float& gapX,
        float& gapY
    ) const
    {
        Base::Vector2d aSk, bSk;
        if (!getConstraintMeasureEndpoints(constraint, aSk, bSk)) {
            return false;
        }
        const auto worldOf = [this](const Base::Vector2d& sk) {
            return mPlane.origin + sk.x * mPlane.xAxis + sk.y * mPlane.yAxis;
        };
        const Eigen::Vector3f wa(
            static_cast<float>(worldOf(aSk).x),
            static_cast<float>(worldOf(aSk).y),
            static_cast<float>(worldOf(aSk).z)
        );
        const Eigen::Vector3f wb(
            static_cast<float>(worldOf(bSk).x),
            static_cast<float>(worldOf(bSk).y),
            static_cast<float>(worldOf(bSk).z)
        );
        const Eigen::Vector2f aS = renderer->worldToScreen(wa);
        const Eigen::Vector2f bS = renderer->worldToScreen(wb);
        float dx = bS.x() - aS.x();
        float dy = bS.y() - aS.y();
        const float len = std::sqrt(dx * dx + dy * dy);
        if (len < 6.0f) {
            return false;
        }
        dx /= len;
        dy /= len;
        const float midX = (aS.x() + bS.x()) * 0.5f;
        const float midY = (aS.y() + bS.y()) * 0.5f;
        const bool isHorizDist = constraint->Type == Sketcher::ConstraintType::DistanceX;
        const bool isVertDist = constraint->Type == Sketcher::ConstraintType::DistanceY;
        const float shaftMargin = 16.0f;
        const float captionGap = 14.0f;
        if (isHorizDist || isVertDist) {
            // Horizontal/vertical distances always draw an axis-aligned
            // dimension shaft instead of a line parallel to the measured
            // segment: DistanceX stays horizontal above the points, DistanceY
            // stays vertical to the right of them.
            if (isHorizDist) {
                const float lineY = std::min(aS.y(), bS.y()) - shaftMargin;
                trackAx = aS.x();
                trackAy = lineY;
                trackBx = bS.x();
                trackBy = lineY;
                gapX = 0.0f;
                gapY = -captionGap;
            }
            else {
                // Vertical distance: shaft to the right of the rightmost point,
                // caption between the measured points and the shaft.
                const float lineX = std::max(aS.x(), bS.x()) + 26.0f;
                trackAx = lineX;
                trackAy = aS.y();
                trackBx = lineX;
                trackBy = bS.y();
                gapX = -captionGap;
                gapY = 0.0f;
            }
            return true;
        }

        // The default caption position (anchor + default pixel offset) tells
        // us which side of the measured geometry the dimension lives on.
        float defDx = 0.0f, defDy = 0.0f;
        defaultLabelOffsetPx(constraint, defDx, defDy);
        const float defaultX = midX + defDx;
        const float defaultY = midY + defDy;

        // Unit normal of the measured segment, oriented towards the default
        // caption position. The shaft is shifted a bit onto that side and the
        // caption keeps a fixed 14 px gap beyond the shaft.
        float nx = -dy;
        float ny = dx;
        float side = (defaultX - midX) * nx + (defaultY - midY) * ny;
        if (side < 0.0f) {
            nx = -nx;
            ny = -ny;
            side = -side;
        }
        float shaftDist = side - captionGap;
        // Keep a clear margin between the dimension shaft and the measured
        // geometry; the caption stays one fixed gap beyond the shaft.
        const float minShaftDist = 16.0f;
        if (shaftDist < minShaftDist) {
            shaftDist = minShaftDist;
        }
        trackAx = aS.x() + nx * shaftDist;
        trackAy = aS.y() + ny * shaftDist;
        trackBx = bS.x() + nx * shaftDist;
        trackBy = bS.y() + ny * shaftDist;
        gapX = nx * captionGap;
        gapY = ny * captionGap;
        return true;
    }
    bool SketcherObj::computeAngleLabelTrack(
        const Sketcher::Constraint* constraint,
        float& centerX,
        float& centerY,
        float& radiusPx,
        float& startDeg,
        float& sweepDeg
    ) const
    {
        if (!constraint || constraint->Type != Sketcher::ConstraintType::Angle) {
            return false;
        }
        Base::Vector2d vertex;
        Base::Vector2d dir1;
        Base::Vector2d dir2;
        bool valid = false;
        Base::Vector2d p1, p2;
        if (constraint->Second == Sketcher::GeoEnum::GeoUndef) {
            // Single line angle against the sketch x-axis: arc center is the
            // line start, sweep goes from the x-axis to the segment direction.
            if (getGeometryPointSketch(constraint->First, PointPos::start, p1)
                && getGeometryPointSketch(constraint->First, PointPos::end, p2)) {
                vertex = p1;
                dir1 = Base::Vector2d(1.0, 0.0);
                dir2 = p2 - p1;
                valid = (dir2.x != 0.0 || dir2.y != 0.0);
            }
        }
        else {
            // Angle between two lines: arc at the intersection, sweeping
            // between the far ends of both segments.
            Base::Vector2d a1, a2, b1, b2;
            if (getGeometryPointSketch(constraint->First, PointPos::start, a1)
                && getGeometryPointSketch(constraint->First, PointPos::end, a2)
                && getGeometryPointSketch(constraint->Second, PointPos::start, b1)
                && getGeometryPointSketch(constraint->Second, PointPos::end, b2)
                && intersectLines2d(a1, a2, b1, b2, vertex)) {
                const auto distSq = [](const Base::Vector2d& a, const Base::Vector2d& b) {
                    const double dx = a.x - b.x;
                    const double dy = a.y - b.y;
                    return dx * dx + dy * dy;
                };
                const Base::Vector2d far1 = distSq(vertex, a1) > distSq(vertex, a2) ? a1 : a2;
                const Base::Vector2d far2 = distSq(vertex, b1) > distSq(vertex, b2) ? b1 : b2;
                dir1 = far1 - vertex;
                dir2 = far2 - vertex;
                valid = (dir1.x != 0.0 || dir1.y != 0.0) && (dir2.x != 0.0 || dir2.y != 0.0);
            }
        }
        if (!valid) {
            return false;
        }
        const auto worldOf = [this](const Base::Vector2d& sk) {
            return mPlane.origin + sk.x * mPlane.xAxis + sk.y * mPlane.yAxis;
        };
        const auto screenOf = [this, &worldOf](const Base::Vector2d& sk) {
            const Base::Vector3d w = worldOf(sk);
            return renderer->worldToScreen(
                Eigen::Vector3f(static_cast<float>(w.x), static_cast<float>(w.y), static_cast<float>(w.z))
            );
        };
        const Eigen::Vector2f vS = screenOf(vertex);
        const Eigen::Vector2f e1S = screenOf(Base::Vector2d(vertex.x + dir1.x, vertex.y + dir1.y));
        const Eigen::Vector2f e2S = screenOf(Base::Vector2d(vertex.x + dir2.x, vertex.y + dir2.y));
        const float pi = 3.14159265358979f;
        const float rawStart = std::atan2(e1S.y() - vS.y(), e1S.x() - vS.x()) * 180.0f / pi;
        const float rawEnd = std::atan2(e2S.y() - vS.y(), e2S.x() - vS.x()) * 180.0f / pi;
        float sweep = rawEnd - rawStart;
        while (sweep > 180.0f) {
            sweep -= 360.0f;
        }
        while (sweep < -180.0f) {
            sweep += 360.0f;
        }
        if (std::fabs(sweep) < 0.5f) {
            return false;
        }
        if (constraint->Second == Sketcher::GeoEnum::GeoUndef) {
            // Half of the segment length, matching the requested annotation.
            const Eigen::Vector2f p1S = screenOf(p1);
            const Eigen::Vector2f p2S = screenOf(p2);
            const float segLen = (p2S - p1S).norm();
            radiusPx = std::max(6.0f, segLen * 0.5f);
        }
        else {
            // Keep the two-line arc compact but inside both rays.
            const float ray1 = (e1S - vS).norm();
            const float ray2 = (e2S - vS).norm();
            const float minRay = std::min(ray1, ray2);
            radiusPx = std::max(10.0f, std::min(30.0f, minRay * 0.4f));
        }
        centerX = vS.x();
        centerY = vS.y();
        startDeg = rawStart;
        sweepDeg = sweep;
        return true;
    }
    Base::Vector2d SketcherObj::constraintLabelAnchor(const Sketcher::Constraint* c) const
    {
        Base::Vector2d p1, p2;
        if (!c) {
            return Base::Vector2d();
        }
        switch (c->Type) {
        case Sketcher::ConstraintType::DistanceX:
        case Sketcher::ConstraintType::DistanceY: {
            if (c->Second != Sketcher::GeoEnum::GeoUndef) {
                if (getGeometryPointSketch(c->First, c->FirstPos, p1)
                    && getGeometryPointSketch(c->Second, c->SecondPos, p2)) {
                    return (p1 + p2) * 0.5;
                }
            }
            else if (c->FirstPos != PointPos::none) {
                // coordinate of a single point
                if (getGeometryPointSketch(c->First, c->FirstPos, p1)) {
                    return p1;
                }
            }
            else if (getGeometryPointSketch(c->First, PointPos::start, p1)
                && getGeometryPointSketch(c->First, PointPos::end, p2)) {
                return (p1 + p2) * 0.5;
            }
            break;
        }
        case Sketcher::ConstraintType::Distance: {
            if (c->Second == Sketcher::GeoEnum::GeoUndef) {
                // length of a single edge: anchor on the middle of its curve
                const Part::Geometry* geo = getGeometry(c->First);
                if (geo) {
                    const auto it = mGeoSegment.find(const_cast<Part::Geometry*>(geo));
                    if (it != mGeoSegment.end() && !it->second.point.empty()) {
                        const Base::Vector3d mid = it->second.point[it->second.point.size() / 2];
                        return Base::Vector2d(mid.x, mid.y);
                    }
                }
            }
            else if (c->FirstPos != PointPos::none && c->SecondPos == PointPos::none) {
                // point -> line distance
                Base::Vector2d la, lb;
                if (getGeometryPointSketch(c->First, c->FirstPos, p1)
                    && getGeometryPointSketch(c->Second, PointPos::start, la)
                    && getGeometryPointSketch(c->Second, PointPos::end, lb)) {
                    Base::Vector2d proj;
                    projectPointOnSegment2d(p1, la, lb, proj);
                    return (p1 + proj) * 0.5;
                }
            }
            else if (c->FirstPos == PointPos::none && c->SecondPos == PointPos::none) {
                // curve -> curve distance
                if (getGeometryCenterSketch(c->First, p1) && getGeometryCenterSketch(c->Second, p2)) {
                    return (p1 + p2) * 0.5;
                }
            }
            else if (c->FirstPos != PointPos::none && c->SecondPos != PointPos::none) {
                // point -> point distance
                if (getGeometryPointSketch(c->First, c->FirstPos, p1)
                    && getGeometryPointSketch(c->Second, c->SecondPos, p2)) {
                    return (p1 + p2) * 0.5;
                }
            }
            break;
        }
        case Sketcher::ConstraintType::Radius:
        case Sketcher::ConstraintType::Diameter: {
            if (getGeometryCenterSketch(c->First, p1)) {
                return p1;
            }
            break;
        }
        case Sketcher::ConstraintType::Angle: {
            if (c->Second == Sketcher::GeoEnum::GeoUndef) {
                // single line against the sketch x-axis: anchor at the start
                // so the caption stays close to the angle arc
                if (getGeometryPointSketch(c->First, PointPos::start, p1)) {
                    return p1;
                }
            }
            else if (c->FirstPos != PointPos::none && c->SecondPos != PointPos::none) {
                if (getGeometryPointSketch(c->First, c->FirstPos, p1)
                    && getGeometryPointSketch(c->Second, c->SecondPos, p2)) {
                    return (p1 + p2) * 0.5;
                }
            }
            else if (c->FirstPos == PointPos::none && c->SecondPos == PointPos::none) {
                // angle between two lines: use their intersection when it is
                // near the segments, otherwise fall back to the middle point
                Base::Vector2d a1, a2, b1, b2;
                if (getGeometryPointSketch(c->First, PointPos::start, a1)
                    && getGeometryPointSketch(c->First, PointPos::end, a2)
                    && getGeometryPointSketch(c->Second, PointPos::start, b1)
                    && getGeometryPointSketch(c->Second, PointPos::end, b2)) {
                    Base::Vector2d inter;
                    if (intersectLines2d(a1, a2, b1, b2, inter)) {
                        return inter;
                    }
                    return ((a1 + a2) * 0.5 + (b1 + b2) * 0.5) * 0.5;
                }
            }
            break;
        }
        default:
            break;
        }
        // Fall back to the first geometry centre so the caption still has a
        // reasonable home even for less common element combinations.
        if (getGeometryCenterSketch(c->First, p1)) {
            return p1;
        }
        return Base::Vector2d();
    }
    std::string SketcherObj::constraintLabelText(const Sketcher::Constraint* c) const
    {
        if (!c) {
            return std::string();
        }
        char buf[96] = { 0 };
        const double value = c->getValue();
        switch (c->Type) {
        case Sketcher::ConstraintType::Radius:
            std::snprintf(buf, sizeof(buf), "R%.2f", value);
            break;
        case Sketcher::ConstraintType::Diameter:
            std::snprintf(buf, sizeof(buf), "D%.2f", value);
            break;
        case Sketcher::ConstraintType::Angle:
            std::snprintf(buf, sizeof(buf), "%.2f", value * 180.0 / 3.14159265358979323846);
            break;
        default:
            std::snprintf(buf, sizeof(buf), "%.2f", value);
            break;
        }
        return std::string(buf);
    }
    bool SketcherObj::constraintInError(int constrId) const
    {
        auto contains = [constrId](const std::vector<int>& list) {
            return std::find(list.begin(), list.end(), constrId) != list.end();
        };
        return contains(lastConflicting) || contains(lastRedundant)
            || contains(lastPartiallyRedundant) || contains(lastMalformedConstraints);
    }
    bool SketcherObj::computeConstraintLabel(
        int constrId,
        Base::Vector2d& anchorSketch,
        float& screenX,
        float& screenY
    ) const
    {
        if (constrId < 0 || constrId >= static_cast<int>(mConstraintList.size())) {
            return false;
        }
        const Sketcher::Constraint* c = mConstraintList[constrId];
        if (!c || !c->isVisible || !isDimensionLabelType(c->Type)) {
            return false;
        }
        if (c->First == Sketcher::GeoEnum::GeoUndef) {
            return false;  // no measured geometry to attach the caption to
        }
        const bool straightDim = c->Type == Sketcher::ConstraintType::Distance
            || c->Type == Sketcher::ConstraintType::DistanceX
            || c->Type == Sketcher::ConstraintType::DistanceY;
        if (straightDim) {
            float trackAx = 0.0f, trackAy = 0.0f;
            float trackBx = 0.0f, trackBy = 0.0f;
            float gapX = 0.0f, gapY = 0.0f;
            if (computeStraightLabelTrack(c, trackAx, trackAy, trackBx, trackBy, gapX, gapY)) {
                double t = 0.5;
                const auto paramIt = m_labelManualParam.find(c);
                if (paramIt != m_labelManualParam.end()) {
                    t = paramIt->second;
                }
                anchorSketch = constraintLabelAnchor(c);
                screenX = trackAx + (trackBx - trackAx) * static_cast<float>(t) + gapX;
                screenY = trackAy + (trackBy - trackAy) * static_cast<float>(t) + gapY;
                return true;
            }
        }
        if (c->Type == Sketcher::ConstraintType::Radius
            || c->Type == Sketcher::ConstraintType::Diameter) {
            // Radial captions stay outside the rim: direction comes from the
            // dragged label (or the default up-right corner), distance is the
            // screen radius plus a small gap.
            Base::Vector2d centerSk;
            const Part::Geometry* geo = getGeometry(c->First);
            double radius = -1.0;
            Base::Vector2d rimDirSk;
            if (getGeometryCenterSketch(c->First, centerSk) && geo
                && (geo->is<Part::GeomCircle>() || geo->is<Part::GeomArcOfCircle>())) {
                radius = geo->is<Part::GeomCircle>()
                    ? static_cast<const Part::GeomCircle*>(geo)->getRadius()
                    : static_cast<const Part::GeomArcOfCircle*>(geo)->getRadius();
                if (geo->is<Part::GeomArcOfCircle>()) {
                    const auto segIt = mGeoSegment.find(const_cast<Part::Geometry*>(geo));
                    if (segIt != mGeoSegment.end() && segIt->second.point.size() > 1) {
                        const Base::Vector3d mid = segIt->second.point[segIt->second.point.size() / 2];
                        const Base::Vector2d toMid(mid.x - centerSk.x, mid.y - centerSk.y);
                        const double dLen = std::sqrt(toMid.x * toMid.x + toMid.y * toMid.y);
                        if (dLen > 1.0e-9) {
                            rimDirSk = Base::Vector2d(toMid.x / dLen, toMid.y / dLen);
                        }
                    }
                }
                else {
                    rimDirSk = Base::Vector2d(0.70710678118, 0.70710678118);
                }
            }
            if (radius > 0.0 && (rimDirSk.x != 0.0 || rimDirSk.y != 0.0)) {
                const Base::Vector3d centerW = mPlane.origin + centerSk.x * mPlane.xAxis
                    + centerSk.y * mPlane.yAxis;
                const Base::Vector3d rimW = centerW + rimDirSk.x * mPlane.xAxis * radius
                    + rimDirSk.y * mPlane.yAxis * radius;
                const Eigen::Vector3f centerF(
                    static_cast<float>(centerW.x),
                    static_cast<float>(centerW.y),
                    static_cast<float>(centerW.z)
                );
                const Eigen::Vector3f rimF(
                    static_cast<float>(rimW.x),
                    static_cast<float>(rimW.y),
                    static_cast<float>(rimW.z)
                );
                const Eigen::Vector2f centerS = renderer->worldToScreen(centerF);
                const Eigen::Vector2f rimS = renderer->worldToScreen(rimF);
                float rx = rimS.x() - centerS.x();
                float ry = rimS.y() - centerS.y();
                const float rimDist = std::sqrt(rx * rx + ry * ry);
                float dirX = 0.70710678118f;
                float dirY = -0.70710678118f;  // up-right default
                const auto manualIt = m_labelManualOffsetPx.find(c);
                if (manualIt != m_labelManualOffsetPx.end()) {
                    const float dx = static_cast<float>(manualIt->second.x);
                    const float dy = static_cast<float>(manualIt->second.y);
                    const float dLen = std::sqrt(dx * dx + dy * dy);
                    if (dLen > 1.0f) {
                        dirX = dx / dLen;
                        dirY = dy / dLen;
                    }
                }
                if (rimDist > 1.0f) {
                    const float labelDist = rimDist + 18.0f;
                    anchorSketch = centerSk;
                    screenX = centerS.x() + dirX * labelDist;
                    screenY = centerS.y() + dirY * labelDist;
                    return true;
                }
            }
        }
        if (c->Type == Sketcher::ConstraintType::Angle) {
            // The caption lives on the annotation arc (slightly outside it so
            // the text box does not cover the arc itself).
            float cx = 0.0f, cy = 0.0f;
            float arcRadius = 0.0f, startDeg = 0.0f, sweepDeg = 0.0f;
            if (computeAngleLabelTrack(c, cx, cy, arcRadius, startDeg, sweepDeg)) {
                double t = 0.5;
                const auto paramIt = m_labelManualParam.find(c);
                if (paramIt != m_labelManualParam.end()) {
                    t = paramIt->second;
                }
                t = std::max(0.0, std::min(1.0, t));
                const float angleDeg = startDeg + sweepDeg * static_cast<float>(t);
                const float angleRad = angleDeg * 3.14159265358979f / 180.0f;
                const float captionRadius = arcRadius + 8.0f;
                anchorSketch = constraintLabelAnchor(c);
                screenX = cx + captionRadius * std::cos(angleRad);
                screenY = cy + captionRadius * std::sin(angleRad);
                return true;
            }
        }
        anchorSketch = constraintLabelAnchor(c);
        const Base::Vector3d world3 = mPlane.origin + anchorSketch.x * mPlane.xAxis
            + anchorSketch.y * mPlane.yAxis;
        const Eigen::Vector3f world(world3.x, world3.y, world3.z);
        const Eigen::Vector2f screen = renderer->worldToScreen(world);
        float dx = 0.0f, dy = 0.0f;
        const auto it = m_labelManualOffsetPx.find(c);
        if (it != m_labelManualOffsetPx.end()) {
            dx = static_cast<float>(it->second.x);
            dy = static_cast<float>(it->second.y);
        }
        else {
            defaultLabelOffsetPx(c, dx, dy);
        }
        screenX = screen.x() + dx;
        screenY = screen.y() + dy;
        return true;
    }
    int SketcherObj::pickConstraintLabelAt(float mouseX, float mouseY) const
    {
        if (!InEdit()) {
            return -1;
        }
        for (int i = static_cast<int>(mConstraintList.size()) - 1; i >= 0; --i) {
            const Sketcher::Constraint* c = mConstraintList[i];
            if (!c || !c->isVisible || !isDimensionLabelType(c->Type)) {
                continue;
            }
            Base::Vector2d anchor;
            float sx = 0.0f, sy = 0.0f;
            if (!computeConstraintLabel(i, anchor, sx, sy)) {
                continue;
            }
            const ImVec2 ts = ImGui::CalcTextSize(constraintLabelText(c).c_str());
            const float padX = 8.0f, padY = 5.0f;
            if (mouseX >= sx - ts.x * 0.5f - padX && mouseX <= sx + ts.x * 0.5f + padX
                && mouseY >= sy - ts.y * 0.5f - padY && mouseY <= sy + ts.y * 0.5f + padY) {
                return i;
            }
        }
        return -1;
    }
    void SketcherObj::drawConstraintLabels()
    {
        if (!InEdit() || mConstraintList.empty()) {
            return;
        }
        // Keep the hover state in sync with the mouse even when the cursor did
        // not move (e.g. the camera was zoomed with the wheel).
        if (m_labelDrag < 0) {
            if (!isHaveActiveHandler) {
                auto [mx, my] = m_sceneView->getInutState().GetMousePosition();
                m_labelHover = pickConstraintLabelAt(static_cast<float>(mx), static_cast<float>(my));
            }
            else {
                m_labelHover = -1;
            }
        }
        ImDrawList* drawList = ImGui::GetForegroundDrawList();
        if (!drawList) {
            return;
        }
        const float thickness = 2.0f;
        const float pi = 3.14159265358979f;
        for (int i = 0; i < static_cast<int>(mConstraintList.size()); ++i) {
            const Sketcher::Constraint* c = mConstraintList[i];
            if (!c || !c->isVisible || !isDimensionLabelType(c->Type)) {
                continue;
            }
            Base::Vector2d anchor;
            float sx = 0.0f, sy = 0.0f;
            if (!computeConstraintLabel(i, anchor, sx, sy)) {
                continue;
            }
            const std::string text = constraintLabelText(c);
            const ImVec2 ts = ImGui::CalcTextSize(text.c_str());
            const bool isError = constraintInError(i);
            const bool hovered = (i == m_labelHover) || (i == m_labelDrag);
            const float padX = 6.0f, padY = 3.0f;
            const ImVec2 boxMin(sx - ts.x * 0.5f - padX, sy - ts.y * 0.5f - padY);
            const ImVec2 boxMax(sx + ts.x * 0.5f + padX, sy + ts.y * 0.5f + padY);
            const ImU32 arrowCol = isError ? IM_COL32(255, 110, 110, 255)
                                           : IM_COL32(255, 255, 0, 255);
            auto screenOf = [&](const Base::Vector2d& sk) -> Eigen::Vector2f {
                return renderer->worldToScreen(mPlane.valueEigen(sk));
            };
            // Straight dimension shaft drawn with the shared double-arrow
            // primitive. The shaft position only depends on the measured
            // geometry, so dragging the caption never moves the shaft.
            const bool straightDim = c->Type == Sketcher::ConstraintType::Distance
                || c->Type == Sketcher::ConstraintType::DistanceX
                || c->Type == Sketcher::ConstraintType::DistanceY;
            if (straightDim) {
                float trackAx = 0.0f, trackAy = 0.0f;
                float trackBx = 0.0f, trackBy = 0.0f;
                float gapX = 0.0f, gapY = 0.0f;
                if (computeStraightLabelTrack(
                        c, trackAx, trackAy, trackBx, trackBy, gapX, gapY
                    )) {
                    const float dx = trackBx - trackAx;
                    const float dy = trackBy - trackAy;
                    const float len = std::sqrt(dx * dx + dy * dy);
                    if (len > 4.0f) {
                        const float angleDeg = -std::atan2(dy, dx) * 180.0f / pi;
                        ImPlotCustom::drawDoubleArrow(
                            ImPlotCustom::Transform(trackAx, trackAy, angleDeg),
                            arrowCol,
                            len,
                            thickness,
                            nullptr
                        );
                    }
                }
            }
            else if (c->Type == Sketcher::ConstraintType::Radius
                || c->Type == Sketcher::ConstraintType::Diameter) {
                // Radial shaft: circles use a 45 degree direction towards the
                // caption, arcs point at the middle of the arc.
                Base::Vector2d centerSk;
                if (getGeometryCenterSketch(c->First, centerSk)) {
                    double radius = 0.0;
                    Base::Vector2d dirSk;
                    const Part::Geometry* geo = getGeometry(c->First);
                    if (geo && (geo->is<Part::GeomCircle>() || geo->is<Part::GeomArcOfCircle>())) {
                        radius = geo->is<Part::GeomCircle>()
                            ? static_cast<const Part::GeomCircle*>(geo)->getRadius()
                            : static_cast<const Part::GeomArcOfCircle*>(geo)->getRadius();
                        if (geo->is<Part::GeomArcOfCircle>()) {
                            const auto it = mGeoSegment.find(const_cast<Part::Geometry*>(geo));
                            if (it != mGeoSegment.end() && it->second.point.size() > 1) {
                                const Base::Vector3d mid = it->second.point[it->second.point.size() / 2];
                                const Base::Vector2d midSk(mid.x, mid.y);
                                const Base::Vector2d d = midSk - centerSk;
                                const double dLen = std::sqrt(d.x * d.x + d.y * d.y);
                                if (dLen > 1.0e-9) {
                                    dirSk = Base::Vector2d(d.x / dLen, d.y / dLen);
                                }
                            }
                        }
                        else {
                            dirSk = Base::Vector2d(0.70710678118, 0.70710678118);
                        }
                    }
                    if (radius > 0.0 && (dirSk.x != 0.0 || dirSk.y != 0.0)) {
                        const Eigen::Vector2f centerS = screenOf(centerSk);
                        const Eigen::Vector2f rimS = screenOf(
                            Base::Vector2d(centerSk.x + dirSk.x * radius, centerSk.y + dirSk.y * radius)
                        );
                        float rx = rimS.x() - centerS.x();
                        float ry = rimS.y() - centerS.y();
                        float rimDist = std::sqrt(rx * rx + ry * ry);
                        if (rimDist > 4.0f) {
                            // Point the radial shaft towards the caption when
                            // the user has dragged it to another side.
                            const float chipDx = sx - centerS.x();
                            const float chipDy = sy - centerS.y();
                            const float chipDist = std::sqrt(chipDx * chipDx + chipDy * chipDy);
                            if (chipDist > 12.0f) {
                                rx = chipDx / chipDist * rimDist;
                                ry = chipDy / chipDist * rimDist;
                            }
                            const float angleDeg = -std::atan2(ry, rx) * 180.0f / pi;
                            // Radius is drawn like a length dimension: a double
                            // arrow shaft that passes through the circle centre
                            // from one rim side to the other. Its direction is
                            // arbitrary and follows the dragged caption.
                            ImPlotCustom::drawDoubleArrow(
                                ImPlotCustom::Transform(
                                    centerS.x() - rx, centerS.y() - ry, angleDeg
                                ),
                                arrowCol,
                                rimDist * 2.0f,
                                thickness,
                                nullptr
                            );
                        }
                    }
                }
            }
            else if (c->Type == Sketcher::ConstraintType::Angle) {
                // Angle arc: center at the line start (single line) or at the
                // line intersection, radius half the segment length for the
                // single-line case. The caption is placed on this arc.
                float cx = 0.0f, cy = 0.0f;
                float arcRadius = 0.0f, startDeg = 0.0f, sweepDeg = 0.0f;
                if (computeAngleLabelTrack(c, cx, cy, arcRadius, startDeg, sweepDeg)) {
                    ImPlotCustom::drawDoubleArcArrow(
                        ImPlotCustom::Transform(cx, cy, startDeg),
                        arrowCol,
                        arcRadius,
                        sweepDeg,
                        thickness,
                        nullptr
                    );
                }
            }
            const ImU32 textCol = isError ? IM_COL32(255, 92, 92, 255)
                                          : IM_COL32(245, 245, 245, 255);
            const ImU32 borderCol = isError ? IM_COL32(255, 120, 120, 220)
                : hovered ? IM_COL32(255, 255, 140, 255) : IM_COL32(255, 255, 255, 42);
            drawList->AddRectFilled(boxMin, boxMax, IM_COL32(24, 24, 30, 178), 4.0f);
            drawList->AddRect(boxMin, boxMax, borderCol, 4.0f);
            drawList->AddText(ImVec2(sx - ts.x * 0.5f, sy - ts.y * 0.5f), textCol, text.c_str());
        }
    }
    void SketcherObj::editConstraintValue(int constrId)
    {
        const Sketcher::Constraint* c = getConstraint(constrId);
        if (!c || !isDimensionLabelType(c->Type)) {
            return;
        }
        const bool isAngle = c->Type == Sketcher::ConstraintType::Angle;
        const double current = isAngle
            ? c->getValue() * 180.0 / 3.14159265358979323846
            : c->getValue();
        const double maxValue = isAngle ? 360.0 : 1.0e9;
        const std::string title = "Edit " + c->typeToString();
        bool ok = false;
        const double entered = QInputDialog::getDouble(
            nullptr,
            QString::fromStdString(title),
            QString::fromStdString(title),
            current,
            0.0,
            maxValue,
            3,
            &ok
        );
        if (!ok || std::abs(entered - current) < 1.0e-9) {
            return;
        }
        const double datum = isAngle ? entered * 3.14159265358979323846 / 180.0 : entered;
        const int err = setDatum(constrId, datum);
        if (err != 0) {
            CORE_ERROR("Constraint datum change failed, solver error code {}", err);
        }
    }
}
