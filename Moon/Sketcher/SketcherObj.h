#pragma once
#include<memory>
#include <unordered_map>
#include "Geometry.h"
#include "TopoShape.h"
#include "base/Tools2D.h"
namespace Part {
	class  Geometry;
}
namespace MOON {
	class SketcherObj {
	public:
		SketcherObj();
		~SketcherObj();
		void setPlane(int p);
		int getPlane();
		void getPlaneNormal(double*p);
		bool InEdit()const;
		void draw();
		void makeDone();
		void addGeometry( std::unique_ptr<Part::Geometry>&ptr);
		int getPickGeoIndex(const Base::Vector2d& pos);
		bool snapPoint(Base::Vector2d& pos);
		bool seekTrimPoints(
			int GeoId,
			const Base::Vector3d& point,
			int& GeoId1,
			Base::Vector3d& intersect1,
			int& GeoId2,
			Base::Vector3d& intersect2,double& u1,double&u2
		);
		void deleteGeometry(int GeoId);
		void replaceGeometry(int oldGeoId, std::unique_ptr<Part::Geometry>& newGeo);
		void replaceGeometries(const std::vector<int>& oldGeoIds, std::vector<std::unique_ptr<Part::Geometry>>& newGeos);
		bool isClosedCurve(const Part::Geometry* geo);
		bool trim(int GeoId,double u1,double u2, const Base::Vector3d& point1, const Base::Vector3d& point2);
		Part::TopoShape toShape() const;
		Base::Matrix4D getplaneTransform();
	private:
		struct CurveSegement;
		Base::Matrix4D updateTransform()const;
		CurveSegement getCurveSegment( Part::Geometry* geo) ;
		int mPlane = 0;
		Base::Matrix4D planeTransform;
		bool isInEdit = true;
		std::vector<std::unique_ptr<Part::Geometry>>mGeoList;
		struct CurveSegement
		{
			std::vector<Base::Vector2d> point;
			std::vector<double> params;
			std::vector<Base::Vector3d>sepoints;
			CurveSegement(const std::vector<Base::Vector2d>& p, const std::vector<double>&u,const std::vector<Base::Vector3d>&se) : point(p), params(u),sepoints(se) {}
			CurveSegement() {}
		};
		std::unordered_map<Part::Geometry*, CurveSegement>mGeoSegment;
	};

}