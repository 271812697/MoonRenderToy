#pragma once
#include<memory>
#include <unordered_map>
#include <chrono>
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
		// Point positions are provided by the ported Sketcher::PointPos
		// (GeoEnum.h); keep a short alias for use inside this class and by
		// code that refers to SketcherObj::PointPos.
		using PointPos = Sketcher::PointPos;
		struct SelectGeoId
		{
			int GeoId;
			PointPos pointPos = PointPos::none;
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
		void setDrawGrid(bool v) { m_drawGrid = v; }
		bool isDrawGrid() const { return m_drawGrid; }
		SketcherPlane2D getPlane();
		void getPlaneNormal(double*p);
		bool InEdit()const;
		void draw();
		// Sketch backdrop (adaptive background grid, infinite X/Y axes, origin
		// marker). Kept separate from the geometry pass so background visuals
		// can be tuned/disabled without touching curve rendering.
		void drawBackground();
		void makeDone();
		int solve(bool updateGeoAfterSolving = true);
		int addGeometry(std::unique_ptr<Part::Geometry>&ptr);
		int addGeometry(Part::Geometry* curve);
		void addGeometry(const std::vector<Part::Geometry*>& curveList);
		Part::Geometry* getGeometry(int GeoId);
		const Part::Geometry* getGeometry(int GeoId) const;
		int getHighestCurveIndex();
		int getPickGeoIndex(const Base::Vector2d& pos, const Base::Matrix4D& viewPortMat);
		SelectGeoId testSelect(const Base::Vector2d& pos);
		std::vector<int> getSelectIds() const;
		void addSelect(int id);
		std::vector<SelectGeoId> getSelectGeoPosIds() const {
			return selectIds;
		}
		
		void removeSelect(const std::vector<int>& idList);
		int getPreselectId()const {return preSelectGeoId.GeoId;}
		SelectGeoId getPreSelectGeoId()const { return preSelectGeoId; }
		bool snapPoint(Base::Vector2d& pos,const std::set<int>&avoid={});
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
		Part::TopoShape getDoneFaceShape() {
			return doneFaceShape;
		}
		Part::TopoShape getDoneWireShape() {
			return doneWireShape;
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
		int getConstraintCount() const { return static_cast<int>(mConstraintList.size()); }
		const Sketcher::Constraint* getConstraint(int index) const;
		// Find an existing constraint with the same type and elements (datum
		// value ignored), so dimensional values can be edited via setDatum().
		int findConstraint(const Sketcher::Constraint* pattern) const;
		// Change the datum of an existing dimensional/tangent/perpendicular
		// constraint and re-solve; on failure the value is rolled back.
		int setDatum(int constrId, double datum);
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
		// Dimension label overlay (P0): every dimensional constraint gets a
		// draggable text caption while the sketch is edited. Double-clicking a
		// caption opens an editor for the datum value.
		void drawConstraintLabels();
		bool computeConstraintLabel(
			int constrId,
			Base::Vector2d& anchorSketch,
			float& screenX,
			float& screenY
		) const;
		int pickConstraintLabelAt(float mouseX, float mouseY) const;
		void editConstraintValue(int constrId);
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
		Part::TopoShape doneWireShape;
		Part::TopoShape doneFaceShape;
		struct CurveSegment;
		void updateGeoSegment(int id);
		void pickGeo();
		void updateConstraintLabelInteraction();
		bool getGeometryPointSketch(int geoId, PointPos pos, Base::Vector2d& out) const;
		bool getGeometryCenterSketch(int geoId, Base::Vector2d& out) const;
		bool getConstraintMeasureEndpoints(
			const Sketcher::Constraint* constraint,
			Base::Vector2d& a,
			Base::Vector2d& b
		) const;
		// Computes the straight dimension shaft (trackA..trackB) in screen
		// space plus the fixed pixel gap that separates the caption from the
		// shaft. Used both for drawing and for constraining label dragging.
		bool computeStraightLabelTrack(
			const Sketcher::Constraint* constraint,
			float& trackAx,
			float& trackAy,
			float& trackBx,
			float& trackBy,
			float& gapX,
			float& gapY
		) const;
		// Computes the angle annotation arc (center, radius, start and sweep
		// in screen degrees). For a single line the center is the line start
		// and the radius is half the line length; the caption can then only
		// slide along this arc.
		bool computeAngleLabelTrack(
			const Sketcher::Constraint* constraint,
			float& centerX,
			float& centerY,
			float& radiusPx,
			float& startDeg,
			float& sweepDeg
		) const;
		Base::Vector2d constraintLabelAnchor(const Sketcher::Constraint* constraint) const;
		std::string constraintLabelText(const Sketcher::Constraint* constraint) const;
		bool constraintInError(int constrId) const;
		void addSelect(SelectGeoId geoId);
		void clearSelect();
		void moveGeo(SelectGeoId geoId,float dx,float dy);
		Base::Matrix4D updateTransform()const;
		Base::Vector2d getMouseHitSketchPlanePoint();
		CurveSegment getCurveSegment( Part::Geometry* geo) ;
		SketcherPlane2D mPlane ;
		Base::Matrix4D planeTransform;
		bool isInEdit = true;
		bool m_drawGrid = true;
		Sketcher::Sketch solvedSketch;
		std::vector<Sketcher::Constraint*> mConstraintList;
		std::vector<std::unique_ptr<Part::Geometry>>mGeoList;
		SelectGeoId preSelectGeoId = {- 1, PointPos::none};
		std::vector<SelectGeoId> selectIds;
		bool hasClickSelected = false;
		bool m_dragSolverInit = false;
		// P0 dimension-label overlay state
		std::unordered_map<const Sketcher::Constraint*, Base::Vector2d> m_labelManualOffsetPx;
		// 0..1 parameter of the caption along the straight dimension shaft
		std::unordered_map<const Sketcher::Constraint*, double> m_labelManualParam;
		int m_labelHover = -1;
		int m_labelDrag = -1;
		Base::Vector2d m_labelDragOffsetPx;
		Base::Vector2d m_labelDragPressPx;
		int m_lastLabelClick = -1;
		std::chrono::steady_clock::time_point m_lastLabelClickTime;
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
			CurveSegment() {}
		};
		std::unordered_map<Part::Geometry*, CurveSegment>mGeoSegment;
	};
}
