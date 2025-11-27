#include "Gizmo/Widgets/RotateCenter.h"
#include "Gizmo/Gizmo.h"
#include "renderer/SceneView.h"

#include "Gizmo/Interactive/ExecuteCommand.h"
#include "Gizmo/Interactive/RenderWindowInteractor.h"
namespace MOON {
	RotateCenter::RotateCenter(const std::string& name, Editor::Panels::SceneView* view) :GizmoWidget(name), m_sceneView(view)
	{
		m_rightButtonPressObserver=this->Interactor->AddObserver(ExecuteCommand::RightButtonPressEvent, this, &RotateCenter::onMouseRightButtonPressed, 0.0f);
		m_rightButtonReleaseObserver=this->Interactor->AddObserver(ExecuteCommand::RightButtonReleaseEvent, this, &RotateCenter::onMouseRightButtonReleased, 0.0f);

	}
	RotateCenter::~RotateCenter()
	{
		delete m_rightButtonPressObserver.command;
		delete m_rightButtonReleaseObserver.command;
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

}