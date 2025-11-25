#include "GizmoWidget.h"
#include "Gizmo/Gizmo.h"
namespace MOON {
	GizmoWidget::GizmoWidget(const std::string& name):mName(name)
	{
		Gizmo::instance().addGizmoWidget(this);;
		renderer = &Gizmo::instance();
	}
	GizmoWidget::~GizmoWidget()
	{
		Gizmo::instance().removeGizmoWidget(this);
	}
	void GizmoWidget::update()
	{
		if (mActive) {
			onUpdate();
		}
	}
	void GizmoWidget::onUpdate()
	{
	}
}