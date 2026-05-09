#include "Gizmo/Widgets/SketchPlane.h"
#include "Gizmo/Gizmo.h"
#include "Gizmo/MathUtil/MathUtil.h"
#include "renderer/SceneView.h"
#include "Gizmo/Interactive/ExecuteCommand.h"
#include "Gizmo/Interactive/RenderWindowInteractor.h"

namespace MOON {
	
	class SketchPlane::SketchPlaneInternal {
	public:
		SketchPlaneInternal(SketchPlane* clip):mSelf(clip) {
			//clickObserver = mSelf->Interactor->AddObserver(ExecuteCommand::LeftButtonReleaseEvent, this, &ClipPlane::ClipPlaneInternal::onMouseLeftClick, 0.0f);		
			mSelf->Interactor;
		}
		~SketchPlaneInternal() {
			//delete clickObserver.command;
			//delete moveObserver.command;
		}
		
		void onMouseLeftClick() {

		}
	private:
		friend class SketchPlane;
		SketchPlane* mSelf = nullptr;

		ExecuteCommandPair clickObserver;
		ExecuteCommandPair moveObserver;
	};

	SketchPlane::SketchPlane(const std::string& name) :GizmoWidget(name)
	, m_internal(new SketchPlaneInternal(this)){
	
	}
	SketchPlane::~SketchPlane()
	{
		delete m_internal;
	}
	void SketchPlane::onUpdate()
	{

		float radius = renderer->pixelsToWorldSize({0,0,0}, 48);
		renderer->drawOneMesh(
			{0,0,0},
			Eigen::Matrix3f::Identity(),
			Eigen::Vector3f{ 0.2f,0.2f,0.2f },
			"GizmoSketchPlane");
		if (renderer->isSelectPolygon("GizmoSketchPlane", "YPlane")) {
			GizmoSketchPlane().setBlockColor(GizmoSketchPlane().getBlockId("YPlane"),{1,1,0,0.7});
		}
		else
		{
			GizmoSketchPlane().setBlockColor(GizmoSketchPlane().getBlockId("YPlane"), { 1,1,1,1 });
		}
		if (renderer->isSelectPolygon("GizmoSketchPlane", "XPlane")) {
			GizmoSketchPlane().setBlockColor(GizmoSketchPlane().getBlockId("XPlane"), { 1,1,0,0.7 });
		}
		else
		{
			GizmoSketchPlane().setBlockColor(GizmoSketchPlane().getBlockId("XPlane"), { 1,1,1,1 });
		}
		if (renderer->isSelectPolygon("GizmoSketchPlane", "ZPlane")) {
			GizmoSketchPlane().setBlockColor(GizmoSketchPlane().getBlockId("ZPlane"), { 1,1,0,0.7 });
		}
		else
		{
			GizmoSketchPlane().setBlockColor(GizmoSketchPlane().getBlockId("ZPlane"), { 1,1,1,1 });
		}
	}



}