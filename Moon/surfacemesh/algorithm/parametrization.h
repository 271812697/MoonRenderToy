#pragma once
#include "surfacemesh/surface_mesh.h"
namespace MOON {

	void harmonic_parameterization(SurfaceMesh& mesh,
		bool use_uniform_weights = false);
	void lscm_parameterization(SurfaceMesh& mesh);
}