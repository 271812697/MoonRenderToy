#pragma once
#include "surfacemesh/surface_mesh.h"
namespace MOON {
	void triangulate(SurfaceMesh& mesh);
	void triangulate(SurfaceMesh& mesh, Face f);
}