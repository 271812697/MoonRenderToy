#pragma once
#include <OvCore/ECS/Actor.h>

#include <OvCore/Resources/Material.h>
#include <OvCore/Rendering/SceneRenderer.h>
#include <OvCore/SceneSystem/SceneManager.h>

namespace OvEditor::Rendering
{

	class GizmoRenderPass : public OvRendering::Core::ARenderPass
	{
	public:
		GizmoRenderPass(OvRendering::Core::CompositeRenderer& p_renderer);
	private:
		virtual void Draw(OvRendering::Data::PipelineState p_pso) override;

	};
}
