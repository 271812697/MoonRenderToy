#pragma once
#include <Core/ECS/Actor.h>
#include <Core/ECS/Components/CAmbientBoxLight.h>
#include <Core/ECS/Components/CAmbientSphereLight.h>
#include <Core/ECS/Components/CModelRenderer.h>
#include <Core/Resources/Material.h>
#include <Core/Rendering/SceneRenderer.h>
#include <Core/SceneSystem/SceneManager.h>

#include "Context.h"
#include "GizmoBehaviour.h"

#include <Rendering/Entities/Camera.h>
#include <Rendering/Features/DebugShapeRenderFeature.h>
#include <Rendering/HAL/Buffer.h>

#include <cstdint>
#include <memory>
#include <vector>

namespace Editor::Rendering
{
	struct PickPassOption
	{
		bool debug = false;

	};
	class PickingRenderPass : public ::Rendering::Core::ARenderPass
	{
	public:
		using PickingResult =
			std::optional<
			std::variant<Tools::Utils::OptRef<::Core::ECS::Actor>,
			Editor::Core::GizmoBehaviour::EDirection>
			>;
		PickingRenderPass(::Rendering::Core::CompositeRenderer& p_renderer);
		PickingResult ReadbackPickingResult(
			const ::Core::SceneSystem::Scene& p_scene,
			uint32_t p_x,
			uint32_t p_y,bool& isSelected
		);

		/** Queues an asynchronous pick at (x, y).
		 *
		 * The request is served by this frame's picking pass: the pixel is copied
		 * into a pixel pack buffer and fetched one or two frames later through
		 * TryConsumePick(), so the CPU never waits for the GPU. The synchronous
		 * path above costs a full pipeline stall (measured ~10 ms per frame here)
		 * and is therefore only used for the click that must be pixel exact.
		 */
		void RequestPick(uint32_t p_x, uint32_t p_y);

		/** Non blocking fetch of a finished asynchronous pick.
		 *
		 * @return true when a readback was consumed. p_outResult is empty when
		 *         the picked pixel contained nothing (background / no gizmo).
		 */
		bool TryConsumePick(
			const ::Core::SceneSystem::Scene& p_scene,
			PickingResult& p_outResult,
			uint32_t& p_outX,
			uint32_t& p_outY
		);

		PickPassOption& GetPickPassOption();
	private:
		PickingResult DecodePickResult(
			const ::Core::SceneSystem::Scene& p_scene,
			const uint8_t p_pixel[4],
			bool& p_isSelected
		);
		void SetupPickReadbacks();
		void IssuePickReadback();

		virtual void Draw(::Rendering::Data::PipelineState p_pso) override;
		void DrawPickableModels(::Rendering::Data::PipelineState p_pso, ::Core::SceneSystem::Scene& p_scene);
		void DrawPickableCameras(::Rendering::Data::PipelineState p_pso, ::Core::SceneSystem::Scene& p_scene);
		void DrawPickableReflectionProbes(::Rendering::Data::PipelineState p_pso, ::Core::SceneSystem::Scene& p_scene);
		void DrawPickableLights(::Rendering::Data::PipelineState p_pso, ::Core::SceneSystem::Scene& p_scene);
		void DrawPickableGizmo(
			::Rendering::Data::PipelineState p_pso,
			const Maths::FVector3& p_position,
			const Maths::FQuaternion& p_rotation,
			Editor::Core::EGizmoOperation p_operation
		);

	private:
		::Rendering::HAL::Framebuffer m_actorPickingFramebuffer;
		::Core::Resources::Material m_actorPickingFallbackMaterial;
		::Core::Resources::Material m_TopoShapePickingFallbackMaterial;
		::Core::Resources::Material m_reflectionProbeMaterial;
		::Core::Resources::Material m_lightMaterial;
		::Core::Resources::Material m_gizmoPickingMaterial;
		PickPassOption mPickOption;

		/** One in flight asynchronous picking readback (RGBA8, 1 pixel). */
		struct PickReadbackSlot
		{
			std::shared_ptr<::Rendering::HAL::Buffer> buffer;
			uint32_t x = 0;
			uint32_t y = 0;
			bool pending = false;
		};
		static constexpr uint32_t kPickReadbackSlotCount = 3;
		static constexpr uint64_t kPickReadbackBytes = 4;

		std::vector<PickReadbackSlot> m_pickReadbackSlots;
		uint32_t m_pickWriteIndex = 0;
		bool m_pickRequestPending = false;
		uint32_t m_pickRequestX = 0;
		uint32_t m_pickRequestY = 0;
	};
}
