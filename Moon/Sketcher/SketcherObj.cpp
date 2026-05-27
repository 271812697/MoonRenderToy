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
        view.GetCamera()->SetSize(100);
        view.GetCamera()->SetProjectionMode(Rendering::Settings::EProjectionMode::ORTHOGRAPHIC);
        float pos=view.GetCamera()->GetFar()/2.0;
        if (mPlane == 0) {
            view.LookAt({0,0,0},{1,0,0}, pos);
        }
        if (mPlane == 1) {
            view.LookAt({ 0,0,0 }, { 0,1,0 }, pos);
        }
        if (mPlane == 2) {
            view.LookAt({ 0,0,0 }, { 0,0,1 }, pos);
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
        auto renderer=&Gizmo::instance();
        if (InEdit()) {
            renderer->pushSize(3);
            renderer->pushColor({ 255,0,0,255 });
            renderer->drawLine2D({ 100,0 }, { -100,0 },static_cast<Plane2D>(mPlane));
            renderer->popColor();
            renderer->pushColor({255,0,255,0});
            renderer->drawLine2D({ 0,100 }, { 0,-100 }, static_cast<Plane2D>(mPlane));
            renderer->popColor();
            //renderer->drawCircle2D(m_internal->centerPoint,m_internal->radius);
            renderer->popSize();
            renderer->pushColor({ 255,255,255,0 });
            renderer->drawPoint2D({ 0,0 }, 10, static_cast<Plane2D>(mPlane));
            renderer->popColor();
        }
        renderer->pushSize(3);
        Eigen::Vector4<uint8_t> pointColor(255, 0, 0, 255);
        float pointSize = 12;
        
        for (auto& it: mGeoSegment) {
            auto& sePoints = it.second.sepoints;
            for (int i = 0;i < sePoints.size();i++) {
                renderer->drawPoint2D({ sePoints[i].x,sePoints[i].y }, pointColor, pointSize, static_cast<Plane2D>(mPlane));
            }
        }  
        for (auto& it : mGeoSegment) {
            if (it.first->isDerivedFrom<Part::GeomCurve>()) {
                auto& seg = it.second;
                for (int i = 0;i < seg.point.size() - 1;i++) {
                    renderer->drawLine2D({ seg.point[i].x,seg.point[i].y }, { seg.point[i + 1].x,seg.point[i + 1].y }, static_cast<Plane2D>(mPlane));
                }
            }
        }
        renderer->popSize();
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
    void SketcherObj::addGeometry(std::unique_ptr<Part::Geometry>& ptr)
    {
        Part::Geometry* geo = ptr.get();
        mGeoSegment[geo]=getCurveSegment(geo);
        mGeoList.push_back(std::move(ptr));
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
    bool SketcherObj::snapPoint(Base::Vector2d& pos, const Base::Matrix4D& viewPortMat)
    {

        Base::Matrix4D trans= viewPortMat*getplaneTransform();
        Base::Vector3d p1 = trans*Base::Vector3d{pos.x,pos.y,0.0};
        double deltaTole = 15.0;
        double minDist = 10000.0;
   
        bool ret = false;
        // 遍历所有几何图元
        for (int i = 0; i < mGeoList.size(); i++) {
            Part::Geometry* geo = mGeoList[i].get();  
            auto& segment = mGeoSegment[geo];
            for (int j = 0;j < segment.sepoints.size();j++) {
				double dist = (p1 -trans*segment.sepoints[j]).Length();
				if (dist < deltaTole && dist < minDist) {
					minDist = dist;
					ret = true;
					pos = { segment.sepoints[j].x, segment.sepoints[j].y };
				}
            }
        }
        if (!ret) {
      
            for (int i = 0; i < mGeoList.size(); i++) {
                Part::Geometry* geo = mGeoList[i].get();
                auto& segment = mGeoSegment[geo];
                if (geo->isDerivedFrom<Part::GeomCurve>()) {
                    for (int j = 0;j < segment.point.size()-1;j++) {
                        double u = 0.0;
                        double dist = pointToSegmentDist(
                            p1,
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
        return ret;
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
    SketcherObj::CurveSegement SketcherObj::getCurveSegment(Part::Geometry* geo) 
    {
		CurveSegement seg;
		CurveConvert::toVector2D(geo, 50, seg.point, seg.params);
        if (geo->isDerivedFrom<Part::GeomCurve>()) {
            if (geo->is<Part::GeomArcOfCircle>()) {
                Part::GeomArcOfCircle* curve = static_cast<Part::GeomArcOfCircle*>(geo);
                seg.sepoints.push_back(curve->getStartPoint());
                seg.sepoints.push_back(curve->getEndPoint());
                seg.sepoints.push_back(curve->getCenter());
            }
            else if (geo->is<Part::GeomLineSegment>()) {
                Part::GeomLineSegment* lineSeg = static_cast<Part::GeomLineSegment*>(geo);
                seg.sepoints.push_back(lineSeg->getStartPoint());
                seg.sepoints.push_back(lineSeg->getEndPoint());
            }
            else if (geo->is<Part::GeomArcOfConic>()) {
                Part::GeomArcOfConic* curve = static_cast<Part::GeomArcOfConic*>(geo);
                seg.sepoints.push_back(curve->getStartPoint());
                seg.sepoints.push_back(curve->getEndPoint());
                seg.sepoints.push_back(curve->getCenter());
            }
            else if (geo->is<Part::GeomCircle>()) {
                Part::GeomCircle* curve = static_cast<Part::GeomCircle*>(geo);
                seg.sepoints.push_back(curve->getCenter());
            }
        }
        else if (geo->is<Part::GeomPoint>()) {
            Base::Vector3d pos = static_cast<Part::GeomPoint*>(geo)->getPoint();
            seg.sepoints.push_back(pos);
        }
        return seg;
    }
}