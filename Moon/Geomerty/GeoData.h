#pragma once
#include "Maths/FVector3.h"
#include <vector>
namespace MOON {
	struct Line { uint32_t I1; uint32_t I2; };
	struct Facet { uint32_t I1; uint32_t I2; uint32_t I3; };
	struct Domain {
		std::vector<Vector3d> points;
		std::vector<Vector3d>normals;
		std::vector<Facet> facets;
	};
}