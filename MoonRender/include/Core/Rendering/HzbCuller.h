#pragma once

#include <cstdint>
#include <unordered_set>
#include <vector>

#include <Core/SceneSystem/BvhService.h>
#include <Maths/FMatrix4.h>
#include <Rendering/Geometry/bvh.h>

namespace Core::Rendering
{
	/** Statistics exposed for debug overlays (ImGui editor, logs, profiling). */
	struct HzbStats
	{
		uint32_t visitedNodes = 0;
		uint32_t culledNodes = 0;
		uint32_t occludedMeshes = 0;
		uint32_t bvhInstances = 0;
		uint32_t gridWidth = 0;
		uint32_t gridHeight = 0;
		/** Depth range of the read back grid (reversed-Z: 0 = far/background). */
		float gridMinDepth = 0.0f;
		float gridMaxDepth = 0.0f;
		float gridMeanDepth = 0.0f;
		float cullTimeMs = 0.0f;
	};

	/** CPU side hierarchical Z-buffer occlusion culler.
	 *
	 * The depth grid is produced by HzbBuildPass from the *previous* frame
	 * (one frame of latency avoids any GPU->CPU synchronization). The test walks
	 * the scene BVH and prunes whole subtrees whose AABB is fully behind the
	 * occluder depth, which is what makes it cheap for scenes with thousands of
	 * small parts hidden behind a large one.
	 */
	class HzbCuller
	{
	public:
		/** Depth grid (max depth per texel, NDC [0,1], GL bottom-left origin). */
		void SetGrid(uint32_t p_width, uint32_t p_height, std::vector<float> p_depths);
		void ClearGrid();
		bool HasGrid() const { return m_width != 0 && m_height != 0 && !m_depths.empty(); }

		/** Runs one culling pass; call before the filtered drawable list is built. */
		void Cull(
			const ::Core::SceneSystem::BvhService& p_bvhService,
			const Maths::FMatrix4& p_viewProjection,
			uint32_t p_viewportWidth,
			uint32_t p_viewportHeight
		);

		/** True when this mesh instance was found fully hidden in the last pass.
		 *
		 * Meshes that are not part of the scene BVH (e.g. line meshes, the
		 * skysphere) are never reported as occluded.
		 */
		bool IsMeshOccluded(const ::Rendering::Resources::Mesh* p_mesh) const;

		const HzbStats& GetStats() const { return m_stats; }
		void SetDepthBias(float p_bias) { m_depthBias = p_bias; }
		float GetDepthBias() const { return m_depthBias; }

	private:
		bool IsNodeOccluded(const ::Rendering::Geometry::Bvh::Node& p_node) const;

		std::vector<float> m_depths;
		uint32_t m_width = 0;
		uint32_t m_height = 0;

		Maths::FMatrix4 m_viewProjection = Maths::FMatrix4::Identity;
		uint32_t m_viewportWidth = 1;
		uint32_t m_viewportHeight = 1;
		float m_depthBias = 0.0005f;

		std::unordered_set<const ::Rendering::Resources::Mesh*> m_occludedMeshes;
		HzbStats m_stats;
	};
}
