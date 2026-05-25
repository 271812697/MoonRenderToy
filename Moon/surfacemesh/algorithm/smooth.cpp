#pragma once
#include "surfacemesh/algorithm/smooth.h"
#include "surfacemesh/algorithm/util.h"

namespace MOON {


	void explicit_smoothing(SurfaceMesh& mesh, unsigned int iterations, bool use_uniform_laplace)
	{
		if (!mesh.n_vertices())
			return;

		// Laplace matrix (clamp negative cotan weights to zero)
		Eigen::SparseMatrix<double> L;
		if (use_uniform_laplace)
			uniform_laplace_matrix(mesh, L);
		else
			laplace_matrix(mesh, L, true);

		// normalize each row by sum of weights
		// scale by 0.5 to make it more robust
		// multiply by -1 to make it neg. definite again
		L = -0.5 * L.diagonal().asDiagonal().inverse() * L;

		// cancel out boundary vertices
		Eigen::SparseMatrix<double> S;
		auto is_inner = [&](Vertex v) { return !mesh.is_boundary(v); };
		selector_matrix(mesh, is_inner, S);
		L = S.transpose() * S * L;

		// copy vertex coordinates to matrix
		Eigen::MatrixXd X;
		coordinates_to_matrix(mesh, X);

		// perform some iterations
		for (unsigned int i = 0; i < iterations; ++i)
			X += L * X;

		// copy matrix back to vertex coordinates
		matrix_to_coordinates(X, mesh);
	}


	void implicit_smoothing(SurfaceMesh& mesh, Scalar timestep, unsigned int iterations, bool use_uniform_laplace, bool rescale)
	{
		if (!mesh.n_vertices())
			return;

		// store center and area
		Point center_before(0, 0, 0);
		Scalar area_before(0);
		if (rescale)
		{
			center_before = centroid(mesh);
			area_before = surface_area(mesh);
		}

		// build system matrix A (clamp negative cotan weights to zero)
		Eigen::SparseMatrix<double> L;
		Eigen::DiagonalMatrix<double, Eigen::Dynamic> M;
		if (use_uniform_laplace)
		{
			uniform_laplace_matrix(mesh, L);
			uniform_mass_matrix(mesh, M);
		}
		else
		{
			laplace_matrix(mesh, L, true);
			mass_matrix(mesh, M);
		}
		Eigen::SparseMatrix<double> A = Eigen::SparseMatrix<double>(M) - timestep * L;
		Eigen::MatrixXd X, B;

		for (unsigned int iter = 0; iter < iterations; ++iter)
		{
			if (!use_uniform_laplace)
			{
				mass_matrix(mesh, M);
				A = Eigen::SparseMatrix<double>(M) - timestep * L;
			}

			// build right-hand side B
			coordinates_to_matrix(mesh, X);
			B = M * X;

			// solve system
			auto is_constrained = [&](unsigned int i) {
				return mesh.is_boundary(Vertex(i));
				};
			X = cholesky_solve(A, B, is_constrained, X);
			matrix_to_coordinates(X, mesh);

			if (rescale)
			{
				// restore original surface area
				Scalar area_after = surface_area(mesh);
				Scalar scale = sqrt(area_before / area_after);
				for (auto v : mesh.vertices())
					mesh.position(v) *= scale;
				// restore original center
				Point center_after = centroid(mesh);
				Point trans = center_before - center_after;
				for (auto v : mesh.vertices())
					mesh.position(v) += trans;
			}
		}
	}

}