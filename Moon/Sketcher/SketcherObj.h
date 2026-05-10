#pragma once
#include<memory>
#include <vector>
#include "Geometry.h"
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
		void makeDone();
		void addGeometry(std::unique_ptr<Part::Geometry>ptr);
	private:
		int mPlane = 0;
		std::vector<std::unique_ptr<Part::Geometry>>mGeoList;
		
	};

}