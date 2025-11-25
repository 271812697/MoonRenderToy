#include "Gizmo/Widgets/RotateCenter.h"
#include "Gizmo/Gizmo.h"
#include "renderer/SceneView.h"
namespace MOON {
	RotateCenter::RotateCenter(const std::string& name, Editor::Panels::SceneView* view) :GizmoWidget(name), m_sceneView(view)
	{
	}
	RotateCenter::~RotateCenter()
	{
	}
	void RotateCenter::onUpdate()
	{
		auto rc = m_sceneView->GetRoaterCenter();
		Eigen::Vector3f center = { rc.x,rc.y,rc.z };
		renderer->translation(renderer->makeId("rotaterCenter"), center);
	}
}