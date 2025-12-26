#include <algorithm>
#include <thread>
#include <stack>
#include <numeric>
#include <cassert>
#include <vector>
#include <future>
//#include "core/log.h"
#include "Rendering/Geometry/bvh.h"

namespace Rendering::Geometry
{
	static int constexpr kMaxPrimitivesPerLeaf = 1;
	static bool is_nan(float v)
	{
		return v != v;
	}

	void Bvh::Build(bbox const* bounds, int numbounds)
	{
		m_bounds = bbox();
		for (int i = 0; i < numbounds; ++i)
		{
			// Calc bbox
			m_bounds.grow(bounds[i]);
		}
		BuildImpl(bounds, numbounds);
	}

	bbox const& Bvh::Bounds() const
	{
		return m_bounds;
	}

	void  Bvh::InitNodeAllocator(size_t maxnum)
	{
		m_nodecnt = 0;
		m_nodes.resize(maxnum);
	}

	Bvh::Node* Bvh::AllocateNode()
	{
		return &m_nodes[m_nodecnt++];
	}

	void Bvh::BuildNode(SplitRequest const& req, bbox const* bounds, FVector3 const* centroids, int* primindices)
	{
		m_height = std::max(m_height, req.level);

		Node* node = AllocateNode();
		node->bounds = req.bounds;
		node->index = req.index;

		// Create leaf node if we have enough prims
		if (req.numprims < 2)
		{
			node->type = kLeaf;
			node->startidx = static_cast<int>(m_packed_indices.size());
			node->numprims = req.numprims;
			for (auto i = 0; i < req.numprims; ++i)
			{
				m_packed_indices.push_back(primindices[req.startidx + i]);
			}
		}
		else
		{
			// Choose the maximum extent
			int axis = req.centroid_bounds.maxdim();
			float border = req.centroid_bounds.center()[axis];

			if (m_usesah)
			{
				SahSplit ss = FindSahSplit(req, bounds, centroids, primindices);

				if (!is_nan(ss.split))
				{
					axis = ss.dim;
					border = ss.split;

					if (req.numprims < ss.sah && req.numprims < kMaxPrimitivesPerLeaf)
					{
						node->type = kLeaf;
						node->startidx = static_cast<int>(m_packed_indices.size());
						node->numprims = req.numprims;

						for (auto i = 0; i < req.numprims; ++i)
						{
							m_packed_indices.push_back(primindices[req.startidx + i]);
						}

						if (req.ptr) *req.ptr = node;
						return;
					}
				}
			}

			node->type = kInternal;

			// Start partitioning and updating extents for children at the same time
			bbox leftbounds, rightbounds, leftcentroid_bounds, rightcentroid_bounds;
			int splitidx = req.startidx;

			bool near2far = (req.numprims + req.startidx) & 0x1;

			if (req.centroid_bounds.extents()[axis] > 0.f)
			{
				auto first = req.startidx;
				auto last = req.startidx + req.numprims;

				if (near2far)
				{
					while (true)
					{
						while ((first != last) &&
							centroids[primindices[first]][axis] < border)
						{
							leftbounds.grow(bounds[primindices[first]]);
							leftcentroid_bounds.grow(centroids[primindices[first]]);
							++first;
						}

						if (first == last--) break;

						rightbounds.grow(bounds[primindices[first]]);
						rightcentroid_bounds.grow(centroids[primindices[first]]);

						while ((first != last) &&
							centroids[primindices[last]][axis] >= border)
						{
							rightbounds.grow(bounds[primindices[last]]);
							rightcentroid_bounds.grow(centroids[primindices[last]]);
							--last;
						}

						if (first == last) break;

						leftbounds.grow(bounds[primindices[last]]);
						leftcentroid_bounds.grow(centroids[primindices[last]]);

						std::swap(primindices[first++], primindices[last]);
					}
				}
				else
				{
					while (true)
					{
						while ((first != last) &&
							centroids[primindices[first]][axis] >= border)
						{
							leftbounds.grow(bounds[primindices[first]]);
							leftcentroid_bounds.grow(centroids[primindices[first]]);
							++first;
						}

						if (first == last--) break;

						rightbounds.grow(bounds[primindices[first]]);
						rightcentroid_bounds.grow(centroids[primindices[first]]);

						while ((first != last) &&
							centroids[primindices[last]][axis] < border)
						{
							rightbounds.grow(bounds[primindices[last]]);
							rightcentroid_bounds.grow(centroids[primindices[last]]);
							--last;
						}

						if (first == last) break;

						leftbounds.grow(bounds[primindices[last]]);
						leftcentroid_bounds.grow(centroids[primindices[last]]);

						std::swap(primindices[first++], primindices[last]);
					}
				}

				splitidx = first;
			}

			if (splitidx == req.startidx || splitidx == req.startidx + req.numprims)
			{
				splitidx = req.startidx + (req.numprims >> 1);

				for (int i = req.startidx; i < splitidx; ++i)
				{
					leftbounds.grow(bounds[primindices[i]]);
					leftcentroid_bounds.grow(centroids[primindices[i]]);
				}

				for (int i = splitidx; i < req.startidx + req.numprims; ++i)
				{
					rightbounds.grow(bounds[primindices[i]]);
					rightcentroid_bounds.grow(centroids[primindices[i]]);
				}
			}

			// Left request
			SplitRequest leftrequest = { req.startidx, splitidx - req.startidx, &node->lc, leftbounds, leftcentroid_bounds, req.level + 1, (req.index << 1) };
			// Right request
			SplitRequest rightrequest = { splitidx, req.numprims - (splitidx - req.startidx), &node->rc, rightbounds, rightcentroid_bounds, req.level + 1, (req.index << 1) + 1 };

			{
				// Put those to stack
				BuildNode(leftrequest, bounds, centroids, primindices);
			}

			{
				BuildNode(rightrequest, bounds, centroids, primindices);
			}
		}
		// Set parent ptr if any
		if (req.ptr) *req.ptr = node;
	}

	Bvh::SahSplit Bvh::FindSahSplit(SplitRequest const& req, bbox const* bounds, FVector3 const* centroids, int* primindices) const
	{
		// SAH implementation
		// calc centroids histogram
		// int const kNumBins = 128;
		// moving split bin index
		int splitidx = -1;
		// Set SAH to maximum float value as a start
		float sah = std::numeric_limits<float>::max();
		SahSplit split;
		split.dim = 0;
		split.split = std::numeric_limits<float>::quiet_NaN();

		// if we cannot apply histogram algorithm
		// put NAN sentinel as split border
		// PerformObjectSplit simply splits in half
		// in this case
		FVector3 centroid_extents = req.centroid_bounds.extents();
		if (FVector3::Dot(centroid_extents, centroid_extents) == 0.f)
		{
			return split;
		}

		// Bin has bbox and occurence count
		struct Bin
		{
			bbox bounds;
			int count;
		};

		// Keep bins for each dimension
		std::vector<Bin> bins[3];
		bins[0].resize(m_num_bins);
		bins[1].resize(m_num_bins);
		bins[2].resize(m_num_bins);

		// Precompute inverse parent area
		float invarea = 1.f / req.bounds.surface_area();
		// Precompute min point
		FVector3 rootmin = req.centroid_bounds.pmin;

		// Evaluate all dimensions
		for (int axis = 0; axis < 3; ++axis)
		{
			float rootminc = rootmin[axis];
			// Range for histogram
			float centroid_rng = centroid_extents[axis];
			float invcentroid_rng = 1.f / centroid_rng;

			// If the box is degenerate in that dimension skip it
			if (centroid_rng == 0.f) continue;

			// Initialize bins
			for (int i = 0; i < m_num_bins; ++i)
			{
				bins[axis][i].count = 0;
				bins[axis][i].bounds = bbox();
			}

			// Calc primitive refs histogram
			for (int i = req.startidx; i < req.startidx + req.numprims; ++i)
			{
				int idx = primindices[i];
				int binidx = (int)std::min<float>(static_cast<float>(m_num_bins) * ((centroids[idx][axis] - rootminc) * invcentroid_rng), static_cast<float>(m_num_bins - 1));

				++bins[axis][binidx].count;
				bins[axis][binidx].bounds.grow(bounds[idx]);
			}

			std::vector<bbox> rightbounds(m_num_bins - 1);

			// Start with 1-bin right box
			bbox rightbox = bbox();
			for (int i = m_num_bins - 1; i > 0; --i)
			{
				rightbox.grow(bins[axis][i].bounds);
				rightbounds[i - 1] = rightbox;
			}

			bbox leftbox = bbox();
			int  leftcount = 0;
			int  rightcount = req.numprims;

			// Start best SAH search
			// i is current split candidate (split between i and i + 1)
			float sahtmp = 0.f;
			for (int i = 0; i < m_num_bins - 1; ++i)
			{
				leftbox.grow(bins[axis][i].bounds);
				leftcount += bins[axis][i].count;
				rightcount -= bins[axis][i].count;

				// Compute SAH
				sahtmp = m_traversal_cost + (leftcount * leftbox.surface_area() + rightcount * rightbounds[i].surface_area()) * invarea;

				// Check if it is better than what we found so far
				if (sahtmp < sah)
				{
					split.dim = axis;
					splitidx = i;
					split.sah = sah = sahtmp;
				}
			}
		}

		// Choose split plane
		if (splitidx != -1)
		{
			split.split = rootmin[split.dim] + (splitidx + 1) * (centroid_extents[split.dim] / m_num_bins);
		}

		return split;
	}

	void Bvh::BuildImpl(bbox const* bounds, int numbounds)
	{
		if (numbounds == 0) {
			Clear();
			return;
		}
		// Structure describing split request
		InitNodeAllocator(2 * numbounds - 1);
		// Cache some stuff to have faster partitioning
		std::vector<FVector3> centroids(numbounds);
		m_indices.resize(numbounds);
		/*
		Fills the range [first, last) with sequentially increasing values, starting with value and repetitively evaluating ++value.
		*/
		std::iota(m_indices.begin(), m_indices.end(), 0);
		// Calc bbox
		bbox centroid_bounds;
		for (size_t i = 0; i < static_cast<size_t>(numbounds); ++i)
		{
			FVector3 c = bounds[i].center();
			centroid_bounds.grow(c);
			centroids[i] = c;
		}
		SplitRequest init = { 0, numbounds, nullptr, m_bounds, centroid_bounds, 0, 1 };
		BuildNode(init, bounds, &centroids[0], &m_indices[0]);
		// Set root_ pointer
		m_root = &m_nodes[0];
	}

	void Bvh::Clear()
	{
		InitNodeAllocator(0);
		m_nodes.clear();
		m_indices.clear();
		// Identifiers of leaf primitives
		m_indices.clear();
		// Node allocator counter, atomic for thread safety
		m_nodecnt=0;

		// Identifiers of leaf primitives
		m_packed_indices.clear();
		m_root = nullptr;
	}

	void Bvh::PrintStatistics() const
	{
		//LOG_INFO("BVH Statistics:");
		//LOG_INFO("Number of triangles: %d", m_indices.size());
		//LOG_INFO("Number of nodes: %d", m_nodecnt);
		//LOG_INFO("Tree height: %d",GetHeight());
	}

}
