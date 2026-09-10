#include <Core/Rendering/HzbCuller.h>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cfloat>

namespace
{
	// Below this clip-space w the corner is behind (or on) the near plane; such
	// nodes are never culled to stay conservative.
	constexpr float kMinClipW = 1e-5f;
}

namespace Core::Rendering
{
	void HzbCuller::SetGrid(uint32_t p_width, uint32_t p_height, std::vector<float> p_depths)
	{
		if (p_width == 0 || p_height == 0 || p_depths.size() != static_cast<size_t>(p_width) * p_height)
		{
			ClearGrid();
			return;
		}
		m_width = p_width;
		m_height = p_height;
		m_depths = std::move(p_depths);
		m_stats.gridWidth = p_width;
		m_stats.gridHeight = p_height;

		// Grid diagnostics: an all-zero grid (reversed-Z far/background) means the
		// depth resolve or the pyramid reduce produced nothing usable, in which
		// case nothing can ever be culled.
		float minDepth = 1.0f;
		float maxDepth = 0.0f;
		double sum = 0.0;
		for (const float depth : m_depths)
		{
			minDepth = std::min(minDepth, depth);
			maxDepth = std::max(maxDepth, depth);
			sum += depth;
		}
		m_stats.gridMinDepth = minDepth;
		m_stats.gridMaxDepth = maxDepth;
		m_stats.gridMeanDepth = static_cast<float>(
			sum / static_cast<double>(m_depths.size())
		);
	}

	void HzbCuller::ClearGrid()
	{
		m_depths.clear();
		m_width = 0;
		m_height = 0;
		m_stats.gridWidth = 0;
		m_stats.gridHeight = 0;
		m_occludedMeshes.clear();
	}

	bool HzbCuller::IsMeshOccluded(const ::Rendering::Resources::Mesh* p_mesh) const
	{
		return p_mesh != nullptr && m_occludedMeshes.find(p_mesh) != m_occludedMeshes.end();
	}

	void HzbCuller::Cull(
		const ::Core::SceneSystem::BvhService& p_bvhService,
		const Maths::FMatrix4& p_viewProjection,
		uint32_t p_viewportWidth,
		uint32_t p_viewportHeight
	)
	{
		m_stats.visitedNodes = 0;
		m_stats.culledNodes = 0;
		m_stats.occludedMeshes = 0;
		m_occludedMeshes.clear();

		if (!HasGrid())
		{
			return;
		}

		const auto* bvh = p_bvhService.m_sceneBvh;
		if (bvh == nullptr || bvh->m_root == nullptr)
		{
			return;
		}

		const auto& instances = p_bvhService.mSceneMeshInstances;
		const auto& meshes = p_bvhService.mSceneMeshes;
		m_stats.bvhInstances = static_cast<uint32_t>(instances.size());

		m_viewProjection = p_viewProjection;
		m_viewportWidth = std::max(1u, p_viewportWidth);
		m_viewportHeight = std::max(1u, p_viewportHeight);

		const auto startTime = std::chrono::high_resolution_clock::now();

		const int* indices = bvh->GetIndices();
		const int instanceCount = static_cast<int>(instances.size());

		std::vector<const ::Rendering::Geometry::Bvh::Node*> stack;
		stack.push_back(bvh->m_root);

		while (!stack.empty())
		{
			const auto* node = stack.back();
			stack.pop_back();

			if (node == nullptr)
			{
				continue;
			}

			++m_stats.visitedNodes;

			if (IsNodeOccluded(*node))
			{
				// Whole subtree is behind the occluder depth: prune it and mark
				// every mesh instance inside as hidden. Culling granularity is
				// the mesh instance, which is also the drawable granularity
				// (ParseScene emits one drawable per mesh/sub-range).
				++m_stats.culledNodes;
				if (node->type == ::Rendering::Geometry::Bvh::kLeaf && indices != nullptr)
				{
					for (int i = 0; i < node->numprims; ++i)
					{
						const int prim = indices[node->startidx + i];
						if (prim < 0 || prim >= instanceCount)
						{
							continue;
						}
						const int meshID = instances[prim].meshID;
						if (meshID >= 0 && meshID < static_cast<int>(meshes.size()))
						{
							m_occludedMeshes.insert(meshes[meshID]);
						}
					}
				}
				continue;
			}

			if (node->type == ::Rendering::Geometry::Bvh::kLeaf)
			{
				continue;
			}

			stack.push_back(node->lc);
			stack.push_back(node->rc);
		}

		m_stats.occludedMeshes = static_cast<uint32_t>(m_occludedMeshes.size());
		m_stats.cullTimeMs = std::chrono::duration<float, std::milli>(
			std::chrono::high_resolution_clock::now() - startTime
		).count();
	}

	bool HzbCuller::IsNodeOccluded(const ::Rendering::Geometry::Bvh::Node& p_node) const
	{
		if (!HasGrid())
		{
			return false;
		}

		// Reversed-Z: window depth 1 = near, 0 = far/background. The object's
		// closest point is therefore the LARGEST value over its corners.
		float closestDepth = -FLT_MAX;
		float uMin = FLT_MAX;
		float vMin = FLT_MAX;
		float uMax = -FLT_MAX;
		float vMax = -FLT_MAX;

		for (int corner = 0; corner < 8; ++corner)
		{
			const float x = (corner & 1) ? p_node.bounds.pmax.x : p_node.bounds.pmin.x;
			const float y = (corner & 2) ? p_node.bounds.pmax.y : p_node.bounds.pmin.y;
			const float z = (corner & 4) ? p_node.bounds.pmax.z : p_node.bounds.pmin.z;

			const Maths::FVector4 clip = m_viewProjection * Maths::FVector4(x, y, z, 1.0f);
			if (clip.w <= kMinClipW)
			{
				// Straddles the near plane (or behind the eye): keep it.
				return false;
			}

			const float invW = 1.0f / clip.w;
			const float ndcX = clip.x * invW;
			const float ndcY = clip.y * invW;
			// glDepthRange(1, 0) flips the standard NDC->window mapping:
			// near (-1) -> 1, far (+1) -> 0.
			const float depth = 0.5f - clip.z * invW * 0.5f;

			closestDepth = std::max(closestDepth, depth);

			const float u = ndcX * 0.5f + 0.5f;
			const float v = ndcY * 0.5f + 0.5f;
			uMin = std::min(uMin, u);
			uMax = std::max(uMax, u);
			vMin = std::min(vMin, v);
			vMax = std::max(vMax, v);
		}

		if (closestDepth <= 0.0f || closestDepth >= 1.0f)
		{
			// On (or beyond) the near/far plane: keep it.
			return false;
		}

		if (uMax < 0.0f || uMin > 1.0f || vMax < 0.0f || vMin > 1.0f)
		{
			// Fully off screen; frustum culling handles this case.
			return false;
		}

		uMin = std::clamp(uMin, 0.0f, 1.0f);
		uMax = std::clamp(uMax, 0.0f, 1.0f);
		vMin = std::clamp(vMin, 0.0f, 1.0f);
		vMax = std::clamp(vMax, 0.0f, 1.0f);

		int x0 = static_cast<int>(std::floor(uMin * m_width));
		int x1 = static_cast<int>(std::ceil(uMax * m_width)) - 1;
		int y0 = static_cast<int>(std::floor(vMin * m_height));
		int y1 = static_cast<int>(std::ceil(vMax * m_height)) - 1;

		x0 = std::clamp(x0, 0, static_cast<int>(m_width) - 1);
		x1 = std::clamp(x1, 0, static_cast<int>(m_width) - 1);
		y0 = std::clamp(y0, 0, static_cast<int>(m_height) - 1);
		y1 = std::clamp(y1, 0, static_cast<int>(m_height) - 1);

		// Conservative occluder depth: the farthest surface inside the covered
		// tile range, which in reversed-Z is the smallest value. If even that
		// surface is closer (larger value) than the node's closest point,
		// nothing inside the node can be visible.
		float occluderDepth = 1.0f;
		for (int y = y0; y <= y1; ++y)
		{
			const size_t row = static_cast<size_t>(y) * m_width;
			for (int x = x0; x <= x1; ++x)
			{
				occluderDepth = std::min(occluderDepth, m_depths[row + x]);
			}
		}

		return occluderDepth > closestDepth + m_depthBias;
	}
}
