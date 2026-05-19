#pragma once
#include<memory>
#include <vector>
#include "Geometry.h"
#include "TopoShape.h"
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
		void addGeometry(std::unique_ptr<Part::Geometry>ptr);
		Part::TopoShape toShape() const;
	private:
		int mPlane = 0;
		bool isInEdit = true;
		std::vector<std::unique_ptr<Part::Geometry>>mGeoList;
	};

}