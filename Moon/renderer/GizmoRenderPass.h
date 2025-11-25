#pragma once
#include <Core/ECS/Actor.h>

#include <Core/Resources/Material.h>
#include <Core/Rendering/SceneRenderer.h>
#include <Core/SceneSystem/SceneManager.h>

namespace Editor::Rendering
{

	class GizmoRenderPass : public ::Rendering::Core::ARenderPass
	{
	public:
		GizmoRenderPass(::Rendering::Core::CompositeRenderer& p_renderer);
		~GizmoRenderPass();
		void enableGizmoWidget(const std::string& name,bool flag);
	private:
		virtual void Draw(::Rendering::Data::PipelineState p_pso) override;
	private:
		class GizmoRenderPassInternal;
		GizmoRenderPassInternal* mInternal = nullptr;

	};
}
