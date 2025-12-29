#include "Gizmo/Widgets/RotateCenter.h"
#include "Gizmo/Gizmo.h"
#include "renderer/SceneView.h"
#include "Gizmo/Interactive/ExecuteCommand.h"
#include "Gizmo/Interactive/RenderWindowInteractor.h"
#include "Qtimgui/imgui/imgui.h"
#include <Core/ECS/Components/CColorBar.h>
namespace MOON {
	int eid = -1;
	uint64_t actorId = 0;
	
	RotateCenter::RotateCenter(const std::string& name) :GizmoWidget(name)
	{
		m_rightButtonPressObserver=this->Interactor->AddObserver(ExecuteCommand::RightButtonPressEvent, this, &RotateCenter::onMouseRightButtonPressed, 0.0f);
		m_rightButtonReleaseObserver=this->Interactor->AddObserver(ExecuteCommand::RightButtonReleaseEvent, this, &RotateCenter::onMouseRightButtonReleased, 0.0f);
		m_mouseMoveObserver = this->Interactor->AddObserver(ExecuteCommand::MouseMoveEvent,this,&RotateCenter::onMouseMove,0.0f);
	}
	RotateCenter::~RotateCenter()
	{
		delete m_rightButtonPressObserver.command;
		delete m_rightButtonReleaseObserver.command;
		delete m_mouseMoveObserver.command;
	}
	void RotateCenter::onUpdate()
	{
		auto rc = m_sceneView->GetRoaterCenter();
		Eigen::Vector3f center = { rc.x,rc.y,rc.z };
		//drawOneMesh(Eigen::Vector3f & translation, Eigen::Matrix3f & rotation, Eigen::Vector3f & scale, const std::string & mesh, bool longterm = false);
		renderer->drawOneMesh(
			center,
			Eigen::Matrix3f::Identity(),
			Eigen::Vector3f{ 0.1f,0.1f,0.1f },
			"Axis");
	}
	void RotateCenter::onMouseRightButtonPressed()
	{
		setVisible(true);
	}
	void RotateCenter::onMouseRightButtonReleased()
	{
		setVisible(false);
	}

	void RotateCenter::onMouseMove()
	{
		auto ray=m_sceneView->GetMouseRay();
		::Core::SceneSystem::HitRes res;
		if (m_sceneView->GetScene()->RayHit(ray, res)) {
			int id=round(res.hitUv.x);
			if (id != eid) {
				actorId =res.actorId;
				eid = id;
				auto actor = m_sceneView->GetScene()->FindActorByID(actorId);
				if (actor) {
					if (actor->GetTag() == "Geomerty") {
						auto colorBar = actor->GetComponent<::Core::ECS::Components::ColorBar>();
						if (colorBar) {
							colorBar->SetColor(eid, Maths::FVector4{ 1.0f,1.0f,0.0f,1.0f });
						}
					}
				}
			}
		}
	}
}