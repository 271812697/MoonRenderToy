#pragma once
#include <vector>
#include "base/Tools2D.h"
#include "base/Vector3D.h"
namespace MOON {
	struct SketcherPlane2D
	{
		Base::Vector3d xAxis { 1.0,0.0,0.0 };
		Base::Vector3d yAxis { 0,1,0 };
		Base::Vector3d normal{ 0,0,1 };
		Base::Vector3d origin { 0,0,0 };
		Base::Vector3d value(const Base::Vector2d& coord) {
			return origin + coord.x * xAxis + coord.y * yAxis;
		}
		Base::Vector3d value(double x, double y) {
			return origin + x * xAxis + y * yAxis;
		}
		SketcherPlane2D() = default;

	};
}