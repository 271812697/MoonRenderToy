#include <ranges>
#include <Core/ECS/Components/CMaterialRenderer.h>
#include <Core/Rendering/EngineDrawableDescriptor.h>
#include "GizmoRenderPass.h"
#include "Core/Global/ServiceLocator.h"
#include "renderer/DebugSceneRenderer.h"
#include "renderer/SceneView.h"
#include "Gizmo/Gizmo.h"
#include "Settings/DebugSetting.h"
#include "Gizmo/Widgets/RotateCenter.h"
#include "Gizmo/Widgets/Measurement.h"

struct GizmoRenderSettings
{
	bool drawBvh = true;
}gizmoRenderSetting;
class Editor::Rendering::GizmoRenderPass::GizmoRenderPassInternal {
	public:
		GizmoRenderPassInternal(Editor::Rendering::GizmoRenderPass* gizmoPass):
			mSelf(gizmoPass)
		{
			mRotateCenterWidget = new MOON::RotateCenter("RotateCenter", &GetService(Editor::Panels::SceneView));
		    mMeasurementWidget = new MOON::Measurement("Measure", &GetService(Editor::Panels::SceneView));
		}
		~GizmoRenderPassInternal()
		{
			delete mRotateCenterWidget;
			delete mMeasurementWidget;
		}
	private:
		friend  class Editor::Rendering::GizmoRenderPass;
		Editor::Rendering::GizmoRenderPass* mSelf = nullptr;
		MOON::RotateCenter* mRotateCenterWidget = nullptr;
		MOON::Measurement* mMeasurementWidget = nullptr;
};
Editor::Rendering::GizmoRenderPass::GizmoRenderPass(::Rendering::Core::CompositeRenderer& p_renderer)
	: ::Rendering::Core::ARenderPass(p_renderer),mInternal(new Editor::Rendering::GizmoRenderPass::GizmoRenderPassInternal(this))
{
	MOON::DebugSettings::instance().addCallBack("showBvh", "Default", [](MOON::NodeBase* self) {
		gizmoRenderSetting.drawBvh= self->getData<bool>();
	});

}

Editor::Rendering::GizmoRenderPass::~GizmoRenderPass()
{
	if (mInternal) {
		delete mInternal;
	}
}

void Editor::Rendering::GizmoRenderPass::Draw(::Rendering::Data::PipelineState p_pso)
{
		
	auto& debugSceneDescriptor = m_renderer.GetDescriptor<Editor::Rendering::DebugSceneRenderer::DebugSceneDescriptor>();
	auto& view = GetService(Editor::Panels::SceneView);;
	auto& renderer = MOON::Gizmo::instance();
	renderer.newFrame(&view);
	Maths::FVector3 out;
	if (view.MouseHit(out)) {
		renderer.drawPoint({ out.x,out.y,out.z }, 15, Eigen::Vector4<uint8_t>{255,0,255,255});
	}

	if (gizmoRenderSetting.drawBvh) {
		auto sceneBvh = view.GetScene()->GetBvh();
		std::vector<::Rendering::Geometry::Bvh::Node*>stack;
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
	}

	renderer.endFrame();
}
