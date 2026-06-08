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
			uint32_t p_y
		);
		PickPassOption& GetPickPassOption();
	private:
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
	};
}
