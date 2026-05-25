#include "shape_mesh.h"
namespace MOON {
	extern Point centroid(const SurfaceMesh& mesh, Face f);

	void dual(SurfaceMesh& mesh)
	{
		SurfaceMesh tmp;
		auto fvertex = mesh.add_face_property<Vertex>("f:vertex");
		for (auto f : mesh.faces())
			fvertex[f] = tmp.add_vertex(centroid(mesh, f));

		for (auto v : mesh.vertices())
		{
			std::vector<Vertex> vertices;
			for (auto f : mesh.faces(v))
				vertices.push_back(fvertex[f]);

			tmp.add_face(vertices);
		}

		mesh.assign(tmp);
	}
	void project_to_unit_sphere(SurfaceMesh& mesh)
	{
		for (auto v : mesh.vertices())
		{
			auto p = mesh.position(v);
			auto n = norm(p);
			mesh.position(v) = (1.0 / n) * p;
		}
	}
	SurfaceMesh* tetrahedron()
	{
		SurfaceMesh* tetrahedron_mesh = new SurfaceMesh();
		float a = 1.0f / 3.0f;
		float b = sqrt(8.0f / 9.0f);
		float c = sqrt(2.0f / 9.0f);
		float d = sqrt(2.0f / 3.0f);
		auto v0 = tetrahedron_mesh->add_vertex(Point(0, 0, 1));
		auto v1 = tetrahedron_mesh->add_vertex(Point(-c, d, -a));
		auto v2 = tetrahedron_mesh->add_vertex(Point(-c, -d, -a));
		auto v3 = tetrahedron_mesh->add_vertex(Point(b, 0, -a));
		tetrahedron_mesh->add_triangle(v0, v1, v2);
		tetrahedron_mesh->add_triangle(v0, v2, v3);
		tetrahedron_mesh->add_triangle(v0, v3, v1);
		tetrahedron_mesh->add_triangle(v3, v2, v1);
		return tetrahedron_mesh;
	}
	SurfaceMesh* hexahedron()
	{
		SurfaceMesh* hexahedron_mesh = new SurfaceMesh();
		float a = 1.0f / sqrt(3.0f);
		auto v0 = hexahedron_mesh->add_vertex(Point(-a, -a, -a));
		auto v1 = hexahedron_mesh->add_vertex(Point(a, -a, -a));
		auto v2 = hexahedron_mesh->add_vertex(Point(a, a, -a));
		auto v3 = hexahedron_mesh->add_vertex(Point(-a, a, -a));
		auto v4 = hexahedron_mesh->add_vertex(Point(-a, -a, a));
		auto v5 = hexahedron_mesh->add_vertex(Point(a, -a, a));
		auto v6 = hexahedron_mesh->add_vertex(Point(a, a, a));
		auto v7 = hexahedron_mesh->add_vertex(Point(-a, a, a));
		hexahedron_mesh->add_quad(v3, v2, v1, v0);
		hexahedron_mesh->add_quad(v2, v6, v5, v1);
		hexahedron_mesh->add_quad(v5, v6, v7, v4);
		hexahedron_mesh->add_quad(v0, v4, v7, v3);
		hexahedron_mesh->add_quad(v3, v7, v6, v2);
		hexahedron_mesh->add_quad(v1, v5, v4, v0);
		return hexahedron_mesh;
	}
	SurfaceMesh* octahedron()
	{
		SurfaceMesh* octahedron_mesh = hexahedron();
		dual(*octahedron_mesh);
		project_to_unit_sphere(*octahedron_mesh);
		return octahedron_mesh;
	}
	SurfaceMesh* icosahedron()
	{
		SurfaceMesh* icosahedron_mesh = new SurfaceMesh();
		float phi = (1.0f + sqrt(5.0f)) * 0.5f; // golden ratio
		float a = 1.0f;
		float b = 1.0f / phi;
		auto v1 = icosahedron_mesh->add_vertex(Point(0, b, -a));
		auto v2 = icosahedron_mesh->add_vertex(Point(b, a, 0));
		auto v3 = icosahedron_mesh->add_vertex(Point(-b, a, 0));
		auto v4 = icosahedron_mesh->add_vertex(Point(0, b, a));
		auto v5 = icosahedron_mesh->add_vertex(Point(0, -b, a));
		auto v6 = icosahedron_mesh->add_vertex(Point(-a, 0, b));
		auto v7 = icosahedron_mesh->add_vertex(Point(0, -b, -a));
		auto v8 = icosahedron_mesh->add_vertex(Point(a, 0, -b));
		auto v9 = icosahedron_mesh->add_vertex(Point(a, 0, b));
		auto v10 = icosahedron_mesh->add_vertex(Point(-a, 0, -b));
		auto v11 = icosahedron_mesh->add_vertex(Point(b, -a, 0));
		auto v12 = icosahedron_mesh->add_vertex(Point(-b, -a, 0));
		project_to_unit_sphere(*icosahedron_mesh);
		icosahedron_mesh->add_triangle(v3, v2, v1);
		icosahedron_mesh->add_triangle(v2, v3, v4);
		icosahedron_mesh->add_triangle(v6, v5, v4);
		icosahedron_mesh->add_triangle(v5, v9, v4);
		icosahedron_mesh->add_triangle(v8, v7, v1);
		icosahedron_mesh->add_triangle(v7, v10, v1);
		icosahedron_mesh->add_triangle(v12, v11, v5);
		icosahedron_mesh->add_triangle(v11, v12, v7);
		icosahedron_mesh->add_triangle(v10, v6, v3);
		icosahedron_mesh->add_triangle(v6, v10, v12);
		icosahedron_mesh->add_triangle(v9, v8, v2);
		icosahedron_mesh->add_triangle(v8, v9, v11);
		icosahedron_mesh->add_triangle(v3, v6, v4);
		icosahedron_mesh->add_triangle(v9, v2, v4);
		icosahedron_mesh->add_triangle(v10, v3, v1);
		icosahedron_mesh->add_triangle(v2, v8, v1);
		icosahedron_mesh->add_triangle(v12, v10, v7);
		icosahedron_mesh->add_triangle(v8, v11, v7);
		icosahedron_mesh->add_triangle(v6, v12, v5);
		icosahedron_mesh->add_triangle(v11, v9, v5);
		return icosahedron_mesh;
	}
	SurfaceMesh* dodecahedron()
	{
		SurfaceMesh* dodecahedron_mesh = icosahedron();
		dual(*dodecahedron_mesh);
		project_to_unit_sphere(*dodecahedron_mesh);
		return dodecahedron_mesh;
	}
}