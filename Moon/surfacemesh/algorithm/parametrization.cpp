#include "surfacemesh/algorithm/parametrization.h"
#include "surfacemesh/algorithm/util.h"
namespace MOON {
	void map_boundary_to_circle(SurfaceMesh& mesh) {
		// get properties
		auto points = mesh.vertex_property<Point>("v:point");
		auto tex = mesh.vertex_property<TexCoord>("v:tex");
		SurfaceMesh::VertexIterator vit, vend = mesh.vertices_end();
		Vertex vh;
		Halfedge hh;
		std::vector<Vertex> loop;
		// Initialize all texture coordinates to the origin.
		for (auto v : mesh.vertices())
			tex[v] = TexCoord(0.5, 0.5);
		// find 1st boundary vertex
		for (vit = mesh.vertices_begin(); vit != vend; ++vit)
			if (mesh.is_boundary(*vit))
				break;
		// collect boundary loop
		vh = *vit;
		hh = mesh.halfedge(vh);
		do
		{
			loop.push_back(mesh.to_vertex(hh));
			hh = mesh.next_halfedge(hh);
		} while (hh != mesh.halfedge(vh));
		// map boundary loop to unit circle in texture domain
		unsigned int i, n = loop.size();
		Scalar angle, l, length;
		TexCoord t;
		// compute length of boundary loop
		for (i = 0, length = 0.0; i < n; ++i)
			length += distance(points[loop[i]], points[loop[(i + 1) % n]]);
		// map length intervalls to unit circle intervals
		for (i = 0, l = 0.0; i < n;)
		{
			// go from 2pi to 0 to preserve orientation
			angle = 2.0 * 3.14159265359 * (1.0 - l / length);
			t[0] = 0.5 + 0.5 * cosf(angle);
			t[1] = 0.5 + 0.5 * sinf(angle);
			tex[loop[i]] = t;
			++i;
			if (i < n)
			{
				l += distance(points[loop[i]], points[loop[(i + 1) % n]]);
			}
		}
	}
	void harmonic_parameterization(SurfaceMesh& mesh, bool use_uniform_weights)
	{
		map_boundary_to_circle(mesh);
		// get properties
		auto tex = mesh.vertex_property<TexCoord>("v:tex");

		// build system matrix (clamp negative cotan weights to zero)
		Eigen::SparseMatrix<double> L;
		if (use_uniform_weights)
			uniform_laplace_matrix(mesh, L);
		else
			laplace_matrix(mesh, L, true);

		// build right-hand side B and inject boundary constraints
		Eigen::MatrixXd B(mesh.n_vertices(), 2);
		B.setZero();
		for (auto v : mesh.vertices())
			if (mesh.is_boundary(v))
				B.row(v.idx()) = static_cast<Eigen::Vector2d>(tex[v]);

		// solve system
		auto is_constrained = [&](unsigned int i) {
			return mesh.is_boundary(Vertex(i));
			};
		Eigen::MatrixXd X = cholesky_solve(L, B, is_constrained, B);
		// copy solution
		for (auto v : mesh.vertices())
			if (!mesh.is_boundary(v))
				tex[v] = X.row(v.idx());
	}
	void lscm_parameterization(SurfaceMesh& mesh)
	{
	}
}