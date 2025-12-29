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
	bool drawCenter = false;
	bool drawRect = false;
	float sx;
	float sy;
	float ex;
	float ey;
	RotateCenter::RotateCenter(const std::string& name) :GizmoWidget(name)
	{
		m_rightButtonPressObserver=this->Interactor->AddObserver(ExecuteCommand::RightButtonPressEvent, this, &RotateCenter::onMouseRightButtonPressed, 0.0f);
		m_rightButtonReleaseObserver=this->Interactor->AddObserver(ExecuteCommand::RightButtonReleaseEvent, this, &RotateCenter::onMouseRightButtonReleased, 0.0f);
		m_leftButtonPressObserver = this->Interactor->AddObserver(ExecuteCommand::LeftButtonPressEvent, this, &RotateCenter::onMouseLeftButtonPressed, 0.0f);
		m_leftButtonReleaseObserver = this->Interactor->AddObserver(ExecuteCommand::LeftButtonReleaseEvent, this, &RotateCenter::onMouseLeftButtonReleased, 0.0f);
		m_mouseMoveObserver = this->Interactor->AddObserver(ExecuteCommand::MouseMoveEvent,this,&RotateCenter::onMouseMove,0.0f);
	}
	RotateCenter::~RotateCenter()
	{
		delete m_rightButtonPressObserver.command;
		delete m_rightButtonReleaseObserver.command;
		delete m_leftButtonPressObserver.command;
		delete m_leftButtonReleaseObserver.command;
		delete m_mouseMoveObserver.command;
	}
	void RotateCenter::onUpdate()
	{
		if (drawCenter) {
			auto rc = m_sceneView->GetRoaterCenter();
			Eigen::Vector3f center = { rc.x,rc.y,rc.z };
			renderer->drawOneMesh(
				center,
				Eigen::Matrix3f::Identity(),
				Eigen::Vector3f{ 0.1f,0.1f,0.1f },
				"Axis");
		}
		if (drawRect) {
			static ImU32 c1 = ImGui::ColorConvertFloat4ToU32({ 1, 1, 0, 0.3 });
			static ImU32 c2 = ImGui::ColorConvertFloat4ToU32({ 1, 1, 0, 1.0 });
			auto drawList=ImGui::GetForegroundDrawList();
			drawList->AddRectFilled({sx,sy},{ex,ey},c1);
			drawList->AddRect({ sx,sy }, { ex,ey }, c1,0,0,4.0);
		}

	}
	void RotateCenter::onMouseRightButtonPressed()
	{
		drawCenter = true;
	}
	void RotateCenter::onMouseRightButtonReleased()
	{
		drawCenter = false;
	}

	void RotateCenter::onMouseLeftButtonPressed()
	{
		drawRect = true;
		auto it=m_sceneView->getInutState().GetMousePosition();
		sx=it.first;
		sy = it.second;
	}

	void RotateCenter::onMouseLeftButtonReleased()
	{
		drawRect = false;
	}

	void RotateCenter::onMouseMove()
	{
		auto it = m_sceneView->getInutState().GetMousePosition();
		ex = it.first;
		ey = it.second;
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