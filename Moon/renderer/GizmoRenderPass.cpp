#include <ranges>
#include <Core/ECS/Components/CMaterialRenderer.h>
#include <Core/Rendering/EngineDrawableDescriptor.h>
#include "GizmoRenderPass.h"
#include "Core/Global/ServiceLocator.h"
#include "core/SceneSystem/BvhService.h"
#include "renderer/DebugSceneRenderer.h"
#include "renderer/SceneView.h"
#include "Gizmo/Gizmo.h"
#include "Settings/DebugSetting.h"
#include "Gizmo/Widgets/RotateCenter.h"
#include "Gizmo/Widgets/Measurement.h"
#include "Gizmo/Widgets/ClipPlane.h"
#include "Gizmo/Widgets/SplitScreen.h"
#include "Gizmo/Widgets/DrawSketchHandlerCircle.h"
#include "Gizmo/Widgets/DrawSketchHandlerArc.h"
#include "Gizmo/Widgets/DrawSketchHandlerBSpline.h"
#include "Gizmo/Widgets/DrawSketchHandlerRectangle.h"
#include "Qtimgui/imgui/imgui.h"

struct GizmoRenderSettings
{
	bool drawBvh = true;
}gizmoRenderSetting;
class Editor::Rendering::GizmoRenderPass::GizmoRenderPassInternal {
	public:
		GizmoRenderPassInternal(Editor::Rendering::GizmoRenderPass* gizmoPass):
			mSelf(gizmoPass)
		{
			mWidgets["RotateCenter"] = new MOON::RotateCenter("RotateCenter");
			mWidgets["Measure"] = new MOON::Measurement("Measure");
			mWidgets["ClipPlane"] = new MOON::ClipPlane("ClipPlane");
			mWidgets["SplitScreen"] = new MOON::SplitScreen("SplitScreen");
			mWidgets["DrawSketchHandlerCircle"] = new MOON::DrawSketchHandlerCircle("DrawSketchHandlerCircle");
			mWidgets["DrawSketchHandlerArc"] = new MOON::DrawSketchHandlerArc("DrawSketchHandlerArc");
			mWidgets["DrawSketchHandlerBSpline"] = new MOON::DrawSketchHandlerBSpline("DrawSketchHandlerBSpline");
			mWidgets["DrawSketchHandlerRectangle"] = new MOON::DrawSketchHandlerRectangle("DrawSketchHandlerRectangle");
			mWidgets["ClipPlane"]->setActive(false);
		}
		~GizmoRenderPassInternal()
		{
			for (auto it : mWidgets) {
				delete it.second;
			}
		}
		void enableGizmoWidget(const std::string& name, bool flag)
		{
			if (mWidgets.find(name) != mWidgets.end()) {
				mWidgets[name]->setActive(flag);
			}
		}
		bool isEnableGizmoWidget(const std::string&name) {
			if (mWidgets.find(name) != mWidgets.end()) {
				return mWidgets[name]->isActived();
			}
			return false;
		}
		MOON::GizmoWidget* getGizmoWidget(const std::string& name) {
			if (mWidgets.find(name) != mWidgets.end()) {
				return mWidgets[name];
			}
			return nullptr;
		}
	private:
		friend  class Editor::Rendering::GizmoRenderPass;
		Editor::Rendering::GizmoRenderPass* mSelf = nullptr;
		MOON::RotateCenter* mRotateCenterWidget = nullptr;
		MOON::Measurement* mMeasurementWidget = nullptr;
		std::unordered_map<std::string, MOON::GizmoWidget*>mWidgets;
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

void Editor::Rendering::GizmoRenderPass::enableGizmoWidget(const std::string& name, bool flag)
{
	mInternal->enableGizmoWidget(name,flag);
}

bool Editor::Rendering::GizmoRenderPass::isEnableGizmoWidget(const std::string& name)
{
	return mInternal->isEnableGizmoWidget(name);
}

MOON::GizmoWidget* Editor::Rendering::GizmoRenderPass::getGizmoWidget(const std::string& name)
{
	return mInternal->getGizmoWidget(name);
}

std::unordered_map<std::string, MOON::GizmoWidget*>& Editor::Rendering::GizmoRenderPass::getGizmoWidgets()
{
	return mInternal->mWidgets;
}



void Editor::Rendering::GizmoRenderPass::Draw(::Rendering::Data::PipelineState p_pso)
{
	auto& view = GetService(Editor::Panels::SceneView);;
	auto& renderer = MOON::Gizmo::instance();
	renderer.newFrame(&view);
	if (gizmoRenderSetting.drawBvh) {
		auto sceneBvh = view.GetScene()->GetBvh();
		std::vector<::Rendering::Geometry::Bvh::Node*>stack;
		if(sceneBvh!=nullptr)
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
