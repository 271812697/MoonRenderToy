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
	SketcherObj::SketcherObj() :EventWidget("SketcherObj")
    {
        setActive(true);
    }
    SketcherObj::~SketcherObj()
    {
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
        draw();
        
    }
    void SketcherObj::onMouseMove()
    {
        onSketchPosP2 = getMouseHitSketchPlanePoint();
		Base::Vector2d preOnSketchPosMove = onSketchPosMove;
        pickGeo();
        if (!isHaveActiveHandler) {
            if (clickMoveState == MoveGeo&& isInEdit) {
                bool solveS = false;;
                for (int i = 0;i < selectIds.size();i++) {
                    int geoId = selectIds[i].GeoId;
                    mGeoList[geoId]->translate(Base::Vector3d(onSketchPosMove.x - preOnSketchPosMove.x, onSketchPosMove.y - preOnSketchPosMove.y, 0));
                    //updateGeoSegment(geoId);
                    solveS = true;
                }
                if (solveS) {
                    this->solve();
                }
            }
        }
    }
    bool sketchDrawRect = false;
    void SketcherObj::onLeftMousePressed()
    {
        sketchDrawRect = true;
        onSketchPosP1=getMouseHitSketchPlanePoint();
        onSketchPosClicked = onSketchPosP1;
		if (preSelectGeoId.GeoId == -1) {
            pickGeo();
		}
        if (!isHaveActiveHandler) {
            if (clickMoveState == SelectGeo) {
                selectIds.clear();
                if (preSelectGeoId.GeoId != -1) {
                    selectIds.push_back(preSelectGeoId);
                    clickMoveState = MoveGeo;
                }
            }
		    else if(clickMoveState == HasSelectGeo) {
			    clickMoveState = MoveGeo;
		    }
        }
    }
    void SketcherObj::onLeftMouseReleased()
    {
        sketchDrawRect = false;
        onSketchPosP2 = getMouseHitSketchPlanePoint();
        if (!isHaveActiveHandler) {
            if (clickMoveState == SelectGeo) {
	            selectIds.clear();
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
                        selectIds.push_back({i,PointPos::None});
                    }
                    else
                    {
                        for (int j = 0; j < seg.sepoints.size(); j++) {
                            bool flag = seg.sepoints[j].coord.x >= minPt.x && seg.sepoints[j].coord.x <= maxPt.x
                                && seg.sepoints[j].coord.y >= minPt.y && seg.sepoints[j].coord.y <= maxPt.y;
                            if (flag) {
                                selectIds.push_back({ i,seg.sepoints[j].pointPos });
                                break;
                            }
                        }
                    }
			    }
                if (selectIds.size() > 0) {
				    clickMoveState = HasSelectGeo;
                }
            }
		    else if (clickMoveState == MoveGeo)
            {
			    clickMoveState = SelectGeo;
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
    void SketcherObj::draw() {
        
        if (InEdit()) {
            renderer->pushSize(3);
            renderer->pushColor({ 255,0,0,255 });
            renderer->drawLine(mPlane.valueEigen(100,0),mPlane.valueEigen(-100,0));
            renderer->popColor();
            renderer->pushColor({255,0,255,0});
            renderer->drawLine(mPlane.valueEigen(0, 100), mPlane.valueEigen(0, -100));
            renderer->popColor();
            renderer->popSize();
            renderer->pushColor({ 255,255,255,0 });
            renderer->drawPoint(mPlane.valueEigen(0,0));
            renderer->popColor();
        }
        renderer->pushSize(3);
        if (clickMoveState == SelectGeo&& sketchDrawRect&&!isHaveActiveHandler) {
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
                renderer->drawPoint(mPlane.valueEigen(sePoints[i].coord.x, sePoints[i].coord.y), pointSize, pointColor);
            }
        }  
        for (int i = 0;i < mGeoList.size();i++) {
			bool isSelect = false;
			for (int j = 0;j < selectIds.size();j++) {
				if (selectIds[j].GeoId == i) {
					isSelect = true;
					break;
				}
			}
			if (isSelect) {
				renderer->pushColor(selectColor);
			}
			else if (i == preSelectGeoId.GeoId) {
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
        doneFaceShape = toShape();
        GetService(SketchToolbar).disableAllHandlers();
    }
    int SketcherObj::solve(bool updateGeoAfterSolving)
    {
        solvedSketch.resetInitMove();
        std::vector<Part::Geometry*> GeoList;
        for (int i = 0; i < mGeoList.size(); i++) {
            GeoList.push_back(mGeoList[i].get());
        }
        solvedSketch.setUpSketch(
            GeoList, mConstraintList,0);
        solvedSketch.solve();
        std::vector<int>GeoIds;
        for (int i = 0; i < mGeoList.size(); i++) {
            GeoIds.push_back(i);;
        } 
        deleteGeometries(GeoIds);
        mGeoList.clear();
        std::vector<Part::Geometry*> geomlist = solvedSketch.extractGeometry();
        addGeometry(geomlist);
        return 0;
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
    SketcherObj::SelectGeoId SketcherObj::testSelect(const Base::Vector2d& pos, const Base::Matrix4D& viewPortMat)
    {
        Base::Matrix4D trans = viewPortMat * getplaneTransform();
        Base::Vector3d p1 = trans * Base::Vector3d{ pos.x,pos.y,0.0 };
        double deltaTole = 5.0;
        double minDist = 10000.0;
        SelectGeoId ret = {-1,PointPos::None } ;
        // travel all segments
        for (int i = 0; i < mGeoList.size(); i++) {
            Part::Geometry* geo = mGeoList[i].get();
            auto& segment = mGeoSegment[geo];
            for (int j = 0; j < segment.sepoints.size(); j++) {
                double dist = (p1 - trans * segment.sepoints[j].coord).Length();
                if (dist < deltaTole && dist < minDist) {
                    minDist = dist;
                    ret.GeoId=i;
                    ret.pointPos = segment.sepoints[j].pointPos;
                }
            }
        }
        if (ret.GeoId == -1) {
            for (int i = 0; i < mGeoList.size(); i++) {
                Part::Geometry* geo = mGeoList[i].get();
                auto& segment = mGeoSegment[geo];
                if (geo->isDerivedFrom<Part::GeomCurve>()) {
                    for (int j = 0;j < segment.point.size() - 1;j++) {
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
                else if(geo->is<Part::GeomPoint>())
                {
                    Base::Vector3d pp = static_cast<Part::GeomPoint*>(geo)->getPoint();
                    double dist=(p1-trans* pp).Length();
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
    void SketcherObj::addSelect(const std::vector<int>& idList)
    {
        for (int i = 0; i < idList.size(); i++) {
            if (idList[i] < mGeoList.size()) {
                selectIds.push_back({ idList[i],PointPos::None });
            }
        }
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
    bool SketcherObj::snapPoint(Base::Vector2d& pos, const Base::Matrix4D& viewPortMat)
    {
        Base::Matrix4D trans= viewPortMat*getplaneTransform();
        //get the screen pos
        Base::Vector3d screenpPos = trans*Base::Vector3d{pos.x,pos.y,0.0};
        double deltaTole = 10.0;
        double minDist = 10000.0;
        bool ret = false;
        // travel all segments
        for (int i = 0; i < mGeoList.size(); i++) {
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
        std::vector<int>deletePos(mGeoList.size(),0);
		for (int i = 0;i < GeoIds.size();i++) {
			if (GeoIds[i] < mGeoList.size()) {
				deletePos[GeoIds[i]] = 1;
			}
		}
		auto it = mGeoList.begin();
		int index = 0;
		while (it != mGeoList.end()) {
			if (deletePos[index] == 1) {
				mGeoSegment.erase((*it).get());
				it = mGeoList.erase(it);
			}
			else {
				it++;
			}
			index++;
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
    void SketcherObj::setBasedTopoShape(Part::TopoShape topoShape)
    {
        basedTopoShape = topoShape;
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
        Sketcher::Constraint* constNew = constraint.release();
        mConstraintList.push_back(constNew);
        return mConstraintList.size()-1;
    }
    void SketcherObj::addConstraint(Sketcher::ConstraintType constrType, int firstGeoId, Sketcher::PointPos firstPos, int secondGeoId, Sketcher::PointPos secondPos, int thirdGeoId, Sketcher::PointPos thirdPos)
    {
        for (int i = 0; i < mConstraintList.size(); i++) {
            if (
                mConstraintList[i]->Type == constrType &&
                mConstraintList[i]->First == firstGeoId&& 
                mConstraintList[i]->FirstPos == firstPos&&
                mConstraintList[i]->Second == secondGeoId&&
                mConstraintList[i]->SecondPos == secondPos &&
                mConstraintList[i]->Third== thirdGeoId &&
                mConstraintList[i]->ThirdPos== thirdPos
                ) 
            {
                return;
            }
        }
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
    void SketcherObj::updateGeoSegment(int id)
    {
        if (id < mGeoList.size()) {
			mGeoSegment[mGeoList[id].get()] = getCurveSegment(mGeoList[id].get());
        }
    }
    void SketcherObj::pickGeo()
    {
        Maths::FMatrix4 mat = m_sceneView->GetCamera()->GetViewPortMatrix();
        Base::Matrix4D pla(
            mat.data[0], mat.data[1], mat.data[2], mat.data[3],
            mat.data[4], mat.data[5], mat.data[6], mat.data[7],
            mat.data[8], mat.data[9], mat.data[10], mat.data[11],
            mat.data[12], mat.data[13], mat.data[14], mat.data[15]
        );
        onSketchPosMove = getMouseHitSketchPlanePoint();
        preSelectGeoId = testSelect(onSketchPosMove, pla);
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
                seg.sepoints.push_back({ curve->getStartPoint(),PointPos::StartP });
                seg.sepoints.push_back({ curve->getEndPoint() ,PointPos::EndP});
                seg.sepoints.push_back({ curve->getCenter() ,PointPos::CenterP});
            }
            else if (geo->is<Part::GeomLineSegment>()) {
                Part::GeomLineSegment* lineSeg = static_cast<Part::GeomLineSegment*>(geo);
                seg.sepoints.push_back({ lineSeg->getStartPoint(),PointPos::StartP });
                seg.sepoints.push_back({ lineSeg->getEndPoint(),PointPos::EndP });
            }
            else if (geo->is<Part::GeomArcOfConic>()) {
                Part::GeomArcOfConic* curve = static_cast<Part::GeomArcOfConic*>(geo);
                seg.sepoints.push_back({ curve->getStartPoint(),PointPos::StartP });
                seg.sepoints.push_back({ curve->getEndPoint() ,PointPos::EndP });
                seg.sepoints.push_back({ curve->getCenter() ,PointPos::CenterP });
            }
            else if (geo->is<Part::GeomCircle>()) {
                Part::GeomCircle* curve = static_cast<Part::GeomCircle*>(geo);
                seg.sepoints.push_back({ curve->getCenter() ,PointPos::CenterP });
            }
            else if (geo->is<Part::GeomBSplineCurve>()) {
                Part::GeomBSplineCurve* curve = static_cast<Part::GeomBSplineCurve*>(geo);
                std::vector<Base::Vector3d>poles= curve->getPoles();
                for (int i = 0; i < poles.size(); i++) {
                    seg.sepoints.push_back({ poles[i],PointPos::CenterP });
                }
            }
        }
        else if (geo->is<Part::GeomPoint>()) {
            Base::Vector3d pos = static_cast<Part::GeomPoint*>(geo)->getPoint();
            seg.sepoints.push_back({ pos ,PointPos::CenterP });
        }
        return seg;
    }
}