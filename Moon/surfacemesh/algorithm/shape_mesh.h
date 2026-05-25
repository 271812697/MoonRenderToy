#pragma once
#include "surfacemesh/surface_mesh.h"
namespace MOON {
	void dual(SurfaceMesh& mesh);
	void project_to_unit_sphere(SurfaceMesh& mesh);
	SurfaceMesh* tetrahedron();
	SurfaceMesh* hexahedron();
	SurfaceMesh* octahedron();
	SurfaceMesh* icosahedron();
	SurfaceMesh* dodecahedron();
}
