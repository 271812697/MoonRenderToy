#include "GizmoWidget.h"
#include "Gizmo/Gizmo.h"
#include "Gizmo/Interactive/RenderWindowInteractor.h"
#include "Core/Global/ServiceLocator.h"
#include "renderer/SceneView.h"
namespace MOON {
	GizmoWidget::GizmoWidget(const std::string& name):mName(name)
	{
		Gizmo::instance().addGizmoWidget(this);;
		renderer = &Gizmo::instance();
		SetInteractor(RenderWindowInteractor::Instance());
		m_sceneView = &GetService(Editor::Panels::SceneView);
	}
	GizmoWidget::~GizmoWidget()
	{
		Gizmo::instance().removeGizmoWidget(this);
	}
	void GizmoWidget::setActive(bool flag)
	{
		 mActive = flag;
		 onSetActive(flag); 
		 SetEnabled(flag ? 1 : 0);
	}
	void GizmoWidget::setVisible(bool flag)
	{
		mVisible = flag;
	}
	void GizmoWidget::update()
	{
		if (mActive&&mVisible) {
			onUpdate();
		}
	}
	void GizmoWidget::onUpdate()
	{

	}
	void GizmoWidget::onSetActive(bool flag)
	{
	}
}