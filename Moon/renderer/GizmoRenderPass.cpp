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
	auto& debugSceneDescriptor = m_renderer.GetDescriptor<Editor::Rendering::DebugSceneRenderer::DebugSceneDescriptor>();
	auto& view = GetService(Editor::Panels::SceneView);;
	auto& renderer = MOON::Guizmo::instance();
	renderer.newFrame(&view);
	auto& inputState = view.getInutState();
	auto [x,y]=inputState.GetMousePosition();
	auto ray = view.GetCamera()->GetMouseRay(x, y);
	auto rc=view.GetRoaterCenter();
	Eigen::Vector3f center = { rc.x,rc.y,rc.z };
	renderer.translation(renderer.makeId("rotaterCenter"), center);

	Maths::FVector3 out;
	if (view.GetScene()->RayHit(ray, out)) {
		renderer.drawPoint({ out.x,out.y,out.z }, 20, Eigen::Vector4<uint8_t>{255,0,255,255});
	}
	std::vector<::Rendering::Geometry::Bvh::Node*>stack;
	auto sceneBvh = view.GetScene()->GetBvh();
	stack.push_back(sceneBvh->m_root);
	while (!stack.empty()) {
		auto cur = stack.back(); stack.pop_back();
		if (!cur)continue;
		if (cur->type == ::Rendering::Geometry::Bvh::kInternal) {
			if (cur->lc) {
				stack.push_back(cur->lc);
			}
			if (cur->rc) {
				stack.push_back(cur->rc);
			}
		}


		auto pmin = cur->bounds.pmin;
		auto pmax = cur->bounds.pmax;
		renderer.drawAlignedBox({ pmin.x,pmin.y ,pmin.z }, { pmax.x,pmax.y ,pmax.z });
	}
	renderer.endFrame();
}
