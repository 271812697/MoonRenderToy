#include "surfacemesh/algorithm/mesh_triangulate.h"
namespace MOON {
	class Triangulation
	{
	public:
		explicit Triangulation(SurfaceMesh& mesh);
		void triangulate(Face f);
	private:

		Scalar compute_weight(int i, int j, int k) const;

		bool is_edge(Vertex a, Vertex b) const;

		bool insert_edge(int i, int j);

		SurfaceMesh& mesh_;
		VertexProperty<Point> points_;
		std::vector<Halfedge> halfedges_;
		std::vector<Vertex> vertices_;

		std::vector<std::vector<Scalar>> weight_;
		std::vector<std::vector<int>> index_;
	};

	Triangulation::Triangulation(SurfaceMesh& mesh) : mesh_(mesh)
	{
		points_ = mesh_.vertex_property<Point>("v:point");
	}

	void Triangulation::triangulate(Face f)
	{
		// collect polygon halfedges
		Halfedge h0 = mesh_.halfedge(f);
		halfedges_.clear();
		vertices_.clear();
		Halfedge h = h0;
		do
		{
			if (!mesh_.is_manifold(mesh_.to_vertex(h)))
			{
				auto what = std::string{ __func__ } + ": Non-manifold polygon";
				throw InvalidInputException(what);
			}

			halfedges_.emplace_back(h);
			vertices_.emplace_back(mesh_.to_vertex(h));
		} while ((h = mesh_.next_halfedge(h)) != h0);

		// do we have at least four vertices?
		const auto n = halfedges_.size();
		if (n <= 3)
			return;

		// compute minimal triangulation by dynamic programming
		weight_.clear();
		weight_.resize(n,
			std::vector<Scalar>(n, std::numeric_limits<Scalar>::max()));
		index_.clear();
		index_.resize(n, std::vector<int>(n, 0));

		// initialize 2-gons
		for (size_t i = 0; i < n - 1; ++i)
		{
			weight_[i][i + 1] = 0.0;
			index_[i][i + 1] = -1;
		}

		// n-gons with n>2
		for (size_t j = 2; j < n; ++j)
		{
			// for all n-gons [i,i+j]
			for (size_t i = 0; i < n - j; ++i)
			{
				auto k = i + j;
				auto wmin = std::numeric_limits<Scalar>::max();
				auto imin = -1;

				// find best split i < m < i+j
				for (size_t m = i + 1; m < k; ++m)
				{
					Scalar w =
						weight_[i][m] + compute_weight(i, m, k) + weight_[m][k];

					if (w < wmin)
					{
						wmin = w;
						imin = m;
					}
				}

				weight_[i][k] = wmin;
				index_[i][k] = imin;
			}
		}

		// now add triangles to mesh
		std::vector<ivec2> todo;
		todo.reserve(n);
		todo.emplace_back(0, n - 1);
		while (!todo.empty())
		{
			ivec2 tri = todo.back();
			todo.pop_back();
			int start = tri[0];
			int end = tri[1];
			if (end - start < 2)
				continue;
			int split = index_[start][end];

			insert_edge(start, split);
			insert_edge(split, end);

			todo.emplace_back(start, split);
			todo.emplace_back(split, end);
		}

		// clean up
		weight_.clear();
		index_.clear();
		halfedges_.clear();
		vertices_.clear();
	}

	Scalar Triangulation::compute_weight(int i, int j, int k) const
	{
		const Vertex a = vertices_[i];
		const Vertex b = vertices_[j];
		const Vertex c = vertices_[k];

		if (is_edge(a, b) && is_edge(b, c) && is_edge(c, a))
			return std::numeric_limits<Scalar>::max();

		const Point& pa = points_[a];
		const Point& pb = points_[b];
		const Point& pc = points_[c];

		return sqrnorm(cross(pb - pa, pc - pa));
	}

	bool Triangulation::is_edge(Vertex a, Vertex b) const
	{
		return mesh_.find_halfedge(a, b).is_valid();
	}

	bool Triangulation::insert_edge(int i, int j)
	{
		Halfedge h0 = halfedges_[i];
		Halfedge h1 = halfedges_[j];
		Vertex v0 = vertices_[i];
		Vertex v1 = vertices_[j];

		if (mesh_.find_halfedge(v0, v1).is_valid())
		{
			return false;
		}

		{
			Halfedge h = h0;
			do
			{
				h = mesh_.next_halfedge(h);
				if (mesh_.to_vertex(h) == v1)
				{
					mesh_.insert_edge(h0, h);
					return true;
				}
			} while (h != h0);
		}

		{
			Halfedge h = h1;
			do
			{
				h = mesh_.next_halfedge(h);
				if (mesh_.to_vertex(h) == v0)
				{
					mesh_.insert_edge(h1, h);
					return true;
				}
			} while (h != h1);
		}

		return false;
	}


	void triangulate(SurfaceMesh& mesh)
	{
		Triangulation tr(mesh);
		for (auto f : mesh.faces())
			tr.triangulate(f);
	}
	void triangulate(SurfaceMesh& mesh, Face f)
	{
		Triangulation(mesh).triangulate(f);
	}
}