#pragma once
#include <vector>
#include "base/Tools2D.h"
namespace Part {
	class  Geometry;
}
namespace MOON {
	class CurveConvert {
	public:
		static std::vector<Base::Vector2d> toVector2D(const Part::Geometry* geometry, int curvedEdgeCountSegments);
		static void toVector2D(const Part::Geometry* geometry, int curvedEdgeCountSegments, std::vector<Base::Vector2d>& out,std::vector<double>&param);
	};
}