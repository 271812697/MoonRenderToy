#pragma once

#include <cstdint>
#include <functional>
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
		/** Mesh instances judged fully hidden (draw call candidates). */
		uint32_t occludedInstances = 0;
		uint32_t bvhInstances = 0;
		/** Number of BVH nodes whose depth test succeeded (fully occluded). */
		uint32_t occludedNodeTests = 0;
		/** Nodes that are behind the occluder but fail only because of the bias. */
		uint32_t biasRejectedNodes = 0;
		/** Nodes rejected because a covered tile still contains background (depth 0). */
		uint32_t backgroundRejectedNodes = 0;
		uint32_t gridWidth = 0;
		uint32_t gridHeight = 0;
		/** Best (largest) occluderDepth - closestDepth seen this frame. */
		float bestMargin = 0.0f;
		/** Relative bias actually used this frame (0 when it is auto-disabled). */
		float effectiveBias = 0.0f;
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
		/** Identifies one drawable mesh instance.
		 *
		 * A Mesh can be shared by several actors (the same model loaded or
		 * duplicated several times), so the mesh pointer alone cannot be used
		 * to decide occlusion: one hidden instance would cull all the others.
		 */
		struct InstanceKey
		{
			const ::Rendering::Resources::Mesh* mesh = nullptr;
			int64_t actorID = 0;

			bool operator==(const InstanceKey& p_other) const
			{
				return mesh == p_other.mesh && actorID == p_other.actorID;
			}
		};

		struct InstanceKeyHash
		{
			size_t operator()(const InstanceKey& p_key) const
			{
				const size_t meshHash = std::hash<const void*>{}(p_key.mesh);
				const size_t actorHash = std::hash<int64_t>{}(p_key.actorID);
				return meshHash ^ (actorHash + 0x9e3779b97f4a7c15ULL + (meshHash << 6) + (meshHash >> 2));
			}
		};

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
		 * Instances that are not part of the scene BVH (e.g. line meshes, the
		 * skysphere) are never reported as occluded.
		 */
		bool IsOccluded(
			const ::Rendering::Resources::Mesh* p_mesh,
			int64_t p_actorID
		) const;

		const HzbStats& GetStats() const { return m_stats; }
		void SetDepthBias(float p_bias) { m_depthBias = p_bias; }
		float GetDepthBias() const { return m_depthBias; }

		/** Bias used while the camera is static (previous depth is still valid). */
		void SetStaticDepthBias(float p_bias) { m_staticDepthBias = p_bias; }
		float GetStaticDepthBias() const { return m_staticDepthBias; }

	private:
		/** Outcome of testing one BVH node against the depth grid. */
		enum class ENodeTestResult
		{
			NoGrid,
			NearClip,
			OutOfRange,
			Offscreen,
			Background,
			BiasRejected,
			Visible,
			Occluded
		};

		ENodeTestResult TestNode(
			const ::Rendering::Geometry::Bvh::Node& p_node,
			float& p_closestDepth,
			float& p_occluderDepth
		) const;

		std::vector<float> m_depths;
		uint32_t m_width = 0;
		uint32_t m_height = 0;

		Maths::FMatrix4 m_viewProjection = Maths::FMatrix4::Identity;
		uint32_t m_viewportWidth = 1;
		uint32_t m_viewportHeight = 1;
		/** Relative depth bias: fraction of the tested node's depth.
		 *
		 * In window depth (reversed-Z) dw / w equals the relative view-space
		 * separation dd / d, so a fixed fraction stays meaningful at any
		 * distance. An absolute NDC epsilon does not: with a dynamic near plane
		 * it can hide objects that are many units apart in view space.
		 */
		float m_depthBias = 0.0005f;
		float m_staticDepthBias = 1e-6f;
		float m_effectiveDepthBias = 0.0005f;
		Maths::FMatrix4 m_lastCullViewProjection = Maths::FMatrix4::Identity;
		bool m_hasLastCullViewProjection = false;

		std::unordered_set<InstanceKey, InstanceKeyHash> m_occludedInstances;
		/** Scratch buffers reused across frames to avoid per-frame allocations. */
		std::vector<const ::Rendering::Geometry::Bvh::Node*> m_traversalStack;
		std::vector<const ::Rendering::Geometry::Bvh::Node*> m_subtreeStack;
		HzbStats m_stats;
	};
}
