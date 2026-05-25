#pragma once
#include "surfacemesh/surface_mesh.h"
namespace MOON {


void decimate(SurfaceMesh& mesh, unsigned int n_vertices,
              Scalar aspect_ratio = 0.0, Scalar edge_length = 0.0,
              unsigned int max_valence = 0, Scalar normal_deviation = 0.0,
              Scalar hausdorff_error = 0.0, Scalar seam_threshold = 1e-2,
              Scalar seam_angle_deviation = 1);

} // namespace pmp
