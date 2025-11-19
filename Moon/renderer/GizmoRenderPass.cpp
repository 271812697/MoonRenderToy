#include <ranges>
#include <Core/ECS/Components/CMaterialRenderer.h>
#include <Core/Rendering/EngineDrawableDescriptor.h>
#include "GizmoRenderPass.h"
#include "Core/Global/ServiceLocator.h"
#include "renderer/DebugSceneRenderer.h"
#include "renderer/SceneView.h"
#include "Guizmo/Guizmo.h"


Editor::Rendering::GizmoRenderPass::GizmoRenderPass(::Rendering::Core::CompositeRenderer& p_renderer)
	: ::Rendering::Core::ARenderPass(p_renderer)
{

}

void Editor::Rendering::GizmoRenderPass::Draw(::Rendering::Data::PipelineState p_pso)
{
	auto& view = GetService(Editor::Panels::SceneView);;
	MOON::Guizmo::instance().newFrame(&view);
	MOON::Guizmo::instance().endFrame();

}
