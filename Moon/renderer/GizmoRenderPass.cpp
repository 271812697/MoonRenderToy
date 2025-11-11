#include <ranges>
#include <OvCore/ECS/Components/CMaterialRenderer.h>
#include <OvCore/Rendering/EngineDrawableDescriptor.h>
#include "GizmoRenderPass.h"
#include "OvCore/Global/ServiceLocator.h"
#include "renderer/DebugSceneRenderer.h"
#include "renderer/SceneView.h"
#include "Guizmo/Guizmo.h"


OvEditor::Rendering::GizmoRenderPass::GizmoRenderPass(OvRendering::Core::CompositeRenderer& p_renderer)
	:OvRendering::Core::ARenderPass(p_renderer)
{

}

void OvEditor::Rendering::GizmoRenderPass::Draw(OvRendering::Data::PipelineState p_pso)
{
	auto& view = GetService(OvEditor::Panels::SceneView);;
	MOON::Guizmo::instance().newFrame(&view);
	MOON::Guizmo::instance().endFrame();

}
