#pragma once
#include<memory>
#include <unordered_map>
#include "Interactive/EventWidget.h"
#include "TopoShape.h"
#include "Sketcher/SketchePlane2D.h"
#include "Sketcher/Datatypes/Constraint.h"
#include "Sketcher/Datatypes/Sketch.h"

namespace Part {
	class  Geometry;
}
namespace MOON {
	class SketcherObj :public EventWidget
	{
	public:
		enum PointPos
		{
			None = 0,
			StartP,
			EndP,
			CenterP
		};
		struct SelectGeoId
		{
			int GeoId;
			PointPos pointPos = None;
		};
		SketcherObj();
		~SketcherObj();

		virtual void onUpdate()override;
		virtual void onMouseMove()override;
		virtual void onLeftMousePressed()override;
		virtual void onLeftMouseReleased()override;
		virtual void onKeyPress(const std::string& key)override;
		virtual void onKeyRelease(const std::string& key)override;
		void setPlane(const SketcherPlane2D&plane);
		void fitCamera();
		void beginEdit();
		SketcherPlane2D getPlane();
		void getPlaneNormal(double*p);
		bool InEdit()const;
		void draw();
		void makeDone();
		int solve(bool updateGeoAfterSolving = true);
		int addGeometry(std::unique_ptr<Part::Geometry>&ptr);
		int addGeometry(Part::Geometry* curve);
		void addGeometry(const std::vector<Part::Geometry*>& curveList);
		Part::Geometry* getGeometry(int GeoId);
		int getHighestCurveIndex();
		int getPickGeoIndex(const Base::Vector2d& pos, const Base::Matrix4D& viewPortMat);
		SelectGeoId testSelect(const Base::Vector2d& pos, const Base::Matrix4D& viewPortMat);
		std::vector<int> getSelectIds() const;
		void addSelect(int id);
		std::vector<SelectGeoId> getSelectGeoPosIds() const {
			return selectIds;
		}
		
		void removeSelect(const std::vector<int>& idList);
		int getPreselectId()const {return preSelectGeoId.GeoId;}
		bool snapPoint(Base::Vector2d& pos,const Base::Matrix4D& viewPortMat);
		int fillet(int geoId1,int geoId2,const Base::Vector3d& refPnt1,const Base::Vector3d& refPnt2,double radius,bool trim = true,bool createCorner = false,bool chamfer = false);
		bool seekTrimPoints(
			int GeoId,
			const Base::Vector3d& point,
			int& GeoId1,
			Base::Vector3d& intersect1,
			int& GeoId2,
			Base::Vector3d& intersect2,double& u1,double&u2
		);
		void deleteGeometry(int GeoId);
		void deleteGeometries(const std::vector<int>& GeoIds);
		void replaceGeometry(int oldGeoId, std::unique_ptr<Part::Geometry>& newGeo);
		void replaceGeometries(const std::vector<int>& oldGeoIds, std::vector<std::unique_ptr<Part::Geometry>>& newGeos);
		bool isClosedCurve(const Part::Geometry* geo);
		bool trim(int GeoId,double u1,double u2, const Base::Vector3d& point1, const Base::Vector3d& point2);
		// clang-format on
		int addSymmetric(const std::vector<int>& geoIdList,int refGeoId);
		std::vector<Part::Geometry*> getSymmetric(
			const std::vector<int>& geoIdList,
			std::map<int, int>& geoIdMap,
			std::map<int, bool>& isStartEndInverted,
			int refGeoId
		);
		Part::TopoShape toShape() const;
		void setBasedTopoShape(Part::TopoShape topoShape);
		Part::TopoShape getBasedTopoShape() {
			return basedTopoShape;
		}
		Part::TopoShape getDoneFaceShape() {
			return doneFaceShape;
		}
		Base::Matrix4D getplaneTransform();
		Base::Vector3d getPlaneOrigin() {
			return mPlane.origin;
		}
		Base::Vector3d getPlaneXAxis() {
			return mPlane.xAxis;
		}
		Base::Vector3d getPlaneYAxis() {
			return mPlane.yAxis;
		}	
		/// add constraint
		int addConstraint(const Sketcher::Constraint* constraint);
		/// add constraint
		int addConstraint(std::unique_ptr<Sketcher::Constraint> constraint);
		// helper function to create a new constraint and move it to the Constraint Property
		void addConstraint(
			Sketcher::ConstraintType constrType,
			int firstGeoId,
			Sketcher::PointPos firstPos,
			int secondGeoId = Sketcher::GeoEnum::GeoUndef,
			Sketcher::PointPos secondPos = Sketcher::PointPos::none,
			int thirdGeoId = Sketcher::GeoEnum::GeoUndef,
			Sketcher::PointPos thirdPos = Sketcher::PointPos::none
		);
		// creates a new constraint
		std::unique_ptr<Sketcher::Constraint> createConstraint(
			Sketcher::ConstraintType constrType,
			int firstGeoId,
			Sketcher::PointPos firstPos,
			int secondGeoId = Sketcher::GeoEnum::GeoUndef,
			Sketcher::PointPos secondPos = Sketcher::PointPos::none,
			int thirdGeoId = Sketcher::GeoEnum::GeoUndef,
			Sketcher::PointPos thirdPos = Sketcher::PointPos::none
		);
	private:
		void retrieveSolverDiagnostics();
		int lastDoF;
		bool lastHasConflict;
		bool lastHasRedundancies;
		bool lastHasPartialRedundancies;
		bool lastHasMalformedConstraints;
		int lastSolverStatus;
		std::vector<int> lastConflicting;
		std::vector<int> lastRedundant;
		std::vector<int> lastPartiallyRedundant;
		std::vector<int> lastMalformedConstraints;
	private:

		Part::TopoShape basedTopoShape;
		Part::TopoShape doneFaceShape;
		struct CurveSegment;
		void updateGeoSegment(int id);
		void pickGeo();
		void addSelect(SelectGeoId geoId);
		void clearSelect();
		Base::Matrix4D updateTransform()const;
		Base::Vector2d getMouseHitSketchPlanePoint();
		CurveSegment getCurveSegment( Part::Geometry* geo) ;
		SketcherPlane2D mPlane ;
		Base::Matrix4D planeTransform;
		bool isInEdit = true;
		Sketcher::Sketch solvedSketch;
		std::vector<Sketcher::Constraint*> mConstraintList;
		std::vector<std::unique_ptr<Part::Geometry>>mGeoList;
		SelectGeoId preSelectGeoId = {- 1,PointPos::None} ;
		std::vector<SelectGeoId> selectIds;
		bool hasClickSelected = false;
		enum SelectState
		{
			Stop,
			Hot,
			OperationGeo,
			DragRect,
			End
		};
		enum SelectMode {
			OverrideSelect,
			AppendSelect
		};
		bool isHaveActiveHandler = false;
		SelectState selectState = Stop;
		SelectMode selectMode = OverrideSelect;
		Base::Vector2d onSketchPosP1;
		Base::Vector2d onSketchPosClicked;//used for click when select geometry curve
		Base::Vector2d onSketchPosMove;//used for mouse move
		Base::Vector2d onSketchPosP2;

		struct SegPoint
		{
			PointPos pointPos;
			Base::Vector3d coord;
			SegPoint(const Base::Vector3d& c,const PointPos& p):coord(c),pointPos(p) {}
		};
		struct CurveSegment
		{
			std::vector<Base::Vector3d> point;
			std::vector<double> params;
			std::vector<SegPoint>sepoints;
			//CurveSegment(const std::vector<Base::Vector3d>& p, const std::vector<double>&u,const std::vector<Base::Vector3d>&se) :
			//	point(p), params(u),sepoints(se) {
			//}
			CurveSegment() {}
		};
		std::unordered_map<Part::Geometry*, CurveSegment>mGeoSegment;
	};
}