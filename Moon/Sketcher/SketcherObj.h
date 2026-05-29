#pragma once
#include<memory>
#include <unordered_map>
#include "Gizmo/GizmoWidget.h"
#include "TopoShape.h"
#include "base/Tools2D.h"
namespace Part {
	class  Geometry;
}
namespace MOON {
	class SketcherObj :public GizmoWidget
	{
	public:
		SketcherObj();
		~SketcherObj();

		virtual void onUpdate()override;
		virtual void onMouseMove()override;
		virtual void onLeftMousePressed()override;
		virtual void onLeftMouseReleased()override;
		virtual void onKeyPress(const std::string& key)override;
		void setPlane(int p);
		int getPlane();
		void getPlaneNormal(double*p);
		bool InEdit()const;
		void draw();
		void makeDone();
		void addGeometry(std::unique_ptr<Part::Geometry>&ptr);
		void addGeometry(const std::vector<Part::Geometry*>& curveList);
		Part::Geometry* getGeometry(int GeoId);
		int getHighestCurveIndex();
		int getPickGeoIndex(const Base::Vector2d& pos, const Base::Matrix4D& viewPortMat);
		int testSelect(const Base::Vector2d& pos, const Base::Matrix4D& viewPortMat);
		std::vector<int> getSelectIds() const { return selectIds; }
		int getPreselectId()const {return preSelectGeoId;}
		bool snapPoint(Base::Vector2d& pos,const Base::Matrix4D& viewPortMat);
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
		Base::Matrix4D getplaneTransform();
	private:
		struct CurveSegement;
		void updateGeoSegment(int id);
		void pickGeo();
		Base::Matrix4D updateTransform()const;
		Base::Vector2d getMouseHitSketchPlanePoint();
		CurveSegement getCurveSegment( Part::Geometry* geo) ;
		int mPlane = 0;
		Base::Matrix4D planeTransform;
		bool isInEdit = true;
		std::vector<std::unique_ptr<Part::Geometry>>mGeoList;
		int preSelectGeoId = -1;
		std::vector<int> selectIds ;
		bool hasClickSelected = false;
		enum ClickMoveState
		{
			SelectGeo,
			HasSelectGeo,
			MoveGeo,
			End
		};
		bool isHaveActiveHandler = false;
		ClickMoveState clickMoveState = SelectGeo;
		Base::Vector2d onSketchPosP1;
		Base::Vector2d onSketchPosClicked;//used for click when select geometry curve
		Base::Vector2d onSketchPosMove;//used for mouse move
		Base::Vector2d onSketchPosP2;
		struct CurveSegement
		{
			std::vector<Base::Vector3d> point;
			std::vector<double> params;
			std::vector<Base::Vector3d>sepoints;
			CurveSegement(const std::vector<Base::Vector3d>& p, const std::vector<double>&u,const std::vector<Base::Vector3d>&se) : point(p), params(u),sepoints(se) {}
			CurveSegement() {}
		};
		std::unordered_map<Part::Geometry*, CurveSegement>mGeoSegment;
	};
}