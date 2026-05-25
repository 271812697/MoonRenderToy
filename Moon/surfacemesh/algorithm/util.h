#pragma once
#include <Eigen/Sparse>
#include <Eigen/Dense>
#include "surfacemesh/surface_mesh.h"
namespace MOON {
	using Triplet = Eigen::Triplet<double>;

	void compute_virtual_vertex(const Eigen::MatrixXd& poly, Eigen::VectorXd& weights);

	void triangle_laplace_matrix(const Eigen::Vector3d& p0,
		const Eigen::Vector3d& p1,
		const Eigen::Vector3d& p2, Eigen::MatrixXd& Ltri);
	void polygon_laplace_matrix(const Eigen::MatrixXd& polygon, Eigen::MatrixXd& Lpoly);
	void uniform_laplace_matrix(const SurfaceMesh& mesh, Eigen::SparseMatrix<double>& L);
	void laplace_matrix(const SurfaceMesh& mesh, Eigen::SparseMatrix<double>& L, bool clamp);
	void triangle_gradient_matrix(const Eigen::Vector3d& p0,
		const Eigen::Vector3d& p1,
		const Eigen::Vector3d& p2, Eigen::MatrixXd& G);

	void polygon_gradient_matrix(const Eigen::MatrixXd& polygon, Eigen::MatrixXd& Gpoly);
	void gradient_matrix(const SurfaceMesh& mesh, Eigen::SparseMatrix<double>& G);
	double triarea(const Eigen::Vector3d& p0, const Eigen::Vector3d& p1,
		const Eigen::Vector3d& p2);
	void divmass_matrix(const SurfaceMesh& mesh, Eigen::DiagonalMatrix<double, Eigen::Dynamic>& M);
	void divergence_matrix(const SurfaceMesh& mesh, Eigen::SparseMatrix<double>& D);
	void selector_matrix(const SurfaceMesh& mesh,
		const std::function<bool(Vertex)>& is_selected,
		Eigen::SparseMatrix<double>& S);
	void coordinates_to_matrix(const SurfaceMesh& mesh, Eigen::MatrixXd& X);
	void matrix_to_coordinates(const Eigen::MatrixXd& X, SurfaceMesh& mesh);
	Scalar face_area(const SurfaceMesh& mesh, Face f);
	Point centroid(const SurfaceMesh& mesh, Face f);
	Point centroid(const SurfaceMesh& mesh);
	Scalar surface_area(const SurfaceMesh& mesh);
	void uniform_mass_matrix(const SurfaceMesh& mesh, Eigen::DiagonalMatrix<double, Eigen::Dynamic>& M);
	void triangle_mass_matrix(const Eigen::Vector3d& p0, const Eigen::Vector3d& p1,
		const Eigen::Vector3d& p2, Eigen::DiagonalMatrix<double, Eigen::Dynamic>& Mtri);
	void polygon_mass_matrix(const Eigen::MatrixXd& polygon, Eigen::DiagonalMatrix<double, Eigen::Dynamic>& Mpoly);
	void mass_matrix(const SurfaceMesh& mesh, Eigen::DiagonalMatrix<double, Eigen::Dynamic>& M);
	Eigen::MatrixXd cholesky_solve(const Eigen::SparseMatrix<double>& A, const Eigen::MatrixXd& b);

	Eigen::MatrixXd cholesky_solve(
		const Eigen::SparseMatrix<double>& A, const  Eigen::MatrixXd& B,
		const std::function<bool(unsigned int)>& is_constrained,
		const  Eigen::MatrixXd& C);

}
