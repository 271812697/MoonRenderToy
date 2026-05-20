#include "Sketcher/SketcherObj.h"
#include "renderer/SceneView.h"
#include "Gizmo/Gizmo.h"
#include "Core/Global/ServiceLocator.h"
#include "Sketcher/SketcheTool2D.h"
#include <TopoDS.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
#include <ShapeFix_Wire.hxx>
#include <BRep_Builder.hxx>
#include <GeomAPI.hxx>
#include <Geom2dAPI_InterCurveCurve.hxx>
#include <Geom2dAPI_ProjectPointOnCurve.hxx>
namespace MOON {
    SketcherObj::SketcherObj()
    {
    }
    SketcherObj::~SketcherObj()
    {
    }
    void SketcherObj::setPlane(int p)
    {
        mPlane = p;        
        auto& view = GetService(Editor::Panels::SceneView);
        view.GetCameraController().EnableRotate(false);
        if (mPlane == 0) {
            view.LookAt({0,0,0},{1,0,0},1);
        }
        if (mPlane == 1) {
            view.LookAt({ 0,0,0 }, { 0,1,0 }, 1);
        }
        if (mPlane == 2) {
            view.LookAt({ 0,0,0 }, { 0,0,1 }, 1);
        }
        planeTransform = updateTransform();
    }
    int SketcherObj::getPlane()
    {
        return mPlane;
    }
    void SketcherObj::getPlaneNormal(double* p)
    {
        if (mPlane == 0) {
            p[0] = 1;
			p[1] = 0;
            p[2] = 0;
        }
        if (mPlane == 1) {
            p[0] = 0;
            p[1] = 1;
            p[2] = 0;
        }
        if (mPlane == 2) {
            p[0] = 0;
            p[1] = 0;
            p[2] = 1;
        }
    }
    void SketcherObj::draw() {
        if (InEdit()) {
            auto renderer=&Gizmo::instance();

            renderer->pushSize(3);
            renderer->pushColor({ 255,0,0,255 });
            renderer->drawLine2D({ 100,0 }, { -100,0 },static_cast<Plane2D>(mPlane));
            renderer->popColor();
            renderer->pushColor({255,0,255,0});
            renderer->drawLine2D({ 0,100 }, { 0,-100 }, static_cast<Plane2D>(mPlane));
            renderer->popColor();
            //renderer->drawCircle2D(m_internal->centerPoint,m_internal->radius);
            renderer->popSize();

        }
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
    }
    void SketcherObj::addGeometry(std::unique_ptr<Part::Geometry> ptr)
    {
        Part::Geometry* geo = ptr.get();
        mGeoSegment[geo]=CurveConvert::toVector2D(geo, 50);
        mGeoList.push_back(std::move(ptr));
    }
    int SketcherObj::getPickGeoIndex(const Base::Vector2d& pos, const Base::Matrix4D& mat)
    {
        Base::Matrix4D trans = mat * planeTransform;
        Base::Vector3d p1 = trans * Base::Vector3d(pos.x, pos.y, 0);

        int ret = -1;
        double deltaTole = 5.0;
        double minDist = 10000.0;

        // ✅ Lambda：点到线段最短距离（内嵌，无需外部函数）
        auto pointToSegmentDist = [](const Base::Vector3d& p, const Base::Vector3d& s, const Base::Vector3d& e) -> double {
            Base::Vector3d se = e - s;
            Base::Vector3d sp = p - s;
            double t = sp.Dot(se) / se.Dot(se);

            if (t < 0.0)
                return sp.Length();
            if (t > 1.0)
                return (p - e).Length();

            Base::Vector3d proj = s + t * se;
            return (p - proj).Length();
            };

        // 遍历所有几何图元
        for (int i = 0; i < mGeoList.size(); i++) {
            Part::Geometry* geo = mGeoList[i].get();
            auto& segment = mGeoSegment[geo];
            int segCount = segment.size();

            if (segCount < 2)
                continue;

            // 遍历每一段线段 [k] → [k+1]
            for (int k = 0; k < segCount - 1; k++) {
                Base::Vector3d s = trans * Base::Vector3d(segment[k].x, segment[k].y, 0);
                Base::Vector3d e = trans * Base::Vector3d(segment[k + 1].x, segment[k + 1].y, 0);

                // ✅ 使用 Lambda 计算真正的点到线段距离
                double dist = pointToSegmentDist(p1, s, e);

                if (dist < deltaTole && dist < minDist) {
                    minDist = dist;
                    ret = i;
                }
            }
        }

        return ret;
    }
    bool SketcherObj::seekTrimPoints(int geometryIndex, const Base::Vector3d& point, int& geometryIndex1, Base::Vector3d& intersect1, int& geometryIndex2, Base::Vector3d& intersect2)
    {
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
                                p1 = p;
                                geometryIndex1 = id;
                            }
                            param -= period;  // transfer param into the interval (pickedParam
                            // pickedParam+period]
                            if (param < param2) {
                                param2 = param;
                                p2 = p;
                                geometryIndex2 = id;
                            }
                        }
                        else if (param < pickedParam && param > param1) {
                            param1 = param;
                            p1 = p;
                            geometryIndex1 = id;
                        }
                        else if (param > pickedParam && param < param2) {
                            param2 = param;
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

        if (geometryIndex1 < 0 && geometryIndex2 < 0) {
            return false;
        }

        if (geometryIndex1 >= 0) {
            intersect1 = Base::Vector3d(p1.X(), p1.Y(), 0.f);
        }
        if (geometryIndex2 >= 0) {
            intersect2 = Base::Vector3d(p2.X(), p2.Y(), 0.f);
        }
        return true;
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
   //
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
      
        //return result.makeFace();
        return result;
    }
    Base::Matrix4D SketcherObj::getplaneTransform()
    {
        return planeTransform;
    }
    Base::Matrix4D SketcherObj::updateTransform() const
    {
        Base::Matrix4D ret;
        if (mPlane == 0) {
           ret=Base::Matrix4D(
                0.0, 0.0, 1.0, 0.0,
                1.0, 0.0, 0.0, 0.0,
                0.0, 1.0, 0.0, 0.0,
                0.0, 0.0, 0.0, 1.0
            );
        }
        if (mPlane == 1) {
           ret=Base::Matrix4D(
                1.0, 0.0, 0.0, 0.0,
                0.0, 0.0, 1.0, 0.0,
                0.0, 1.0, 0.0, 0.0,
                0.0, 0.0, 0.0, 1.0
            );
        }
        if (mPlane == 2) {
           ret=Base::Matrix4D(
                1.0, 0.0, 0.0, 0.0,
                0.0, 1.0, 0.0, 0.0,
                0.0, 0.0, 1.0, 0.0,
                0.0, 0.0, 0.0, 1.0
            );
        }
        return ret;
    }
}