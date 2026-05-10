#include "GizmoWidget.h"
#include "Gizmo/Gizmo.h"
#include "Gizmo/Interactive/Event.h"
#include "Gizmo/Interactive/ExecuteCommand.h"
#include "Gizmo/Interactive/WidgetCallbackMapper.h"
#include "Gizmo/Interactive/WidgetEvent.h"
#include "Gizmo/Interactive/WidgetEventTranslator.h"
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

		// Define widget events
		this->CallbackMapper->SetCallbackMethod(ExecuteCommand::LeftButtonPressEvent, GizmoEvent::NoModifier, 0,
			0, 0, WidgetEvent::Select, this, GizmoWidget::LeftMousePressed);
		this->CallbackMapper->SetCallbackMethod(ExecuteCommand::LeftButtonReleaseEvent, GizmoEvent::NoModifier, 0,
			0, 0, WidgetEvent::Select3D, this, GizmoWidget::LeftMouseReleased);
		this->CallbackMapper->SetCallbackMethod(ExecuteCommand::RightButtonReleaseEvent, GizmoEvent::NoModifier, 0,
			0, 0, WidgetEvent::Completed, this, GizmoWidget::RightMouseReleased);
		this->CallbackMapper->SetCallbackMethod(ExecuteCommand::RightButtonPressEvent, GizmoEvent::NoModifier, 0,
			0, 0, WidgetEvent::EndSelect, this, GizmoWidget::RightMousePressed);
		this->CallbackMapper->SetCallbackMethod(ExecuteCommand::MouseMoveEvent, GizmoEvent::NoModifier, 0,
			0, 0, WidgetEvent::Move3D, this, GizmoWidget::MouseMove);
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
	void GizmoWidget::onLeftMousePressed()
	{
	}

	void GizmoWidget::onLeftMouseReleased()
	{
	}

	void GizmoWidget::onRightMousePressed()
	{
	}

	void GizmoWidget::onRightMouseReleased()
	{

	}

	void GizmoWidget::onMouseMove()
	{
	}

	void GizmoWidget::LeftMousePressed(AbstractWidget* w)
	{
		GizmoWidget* self = reinterpret_cast<GizmoWidget*>(w);
		self->onLeftMousePressed();
	}

	void GizmoWidget::LeftMouseReleased(AbstractWidget* w)
	{
		GizmoWidget* self = reinterpret_cast<GizmoWidget*>(w);
		self->onLeftMouseReleased();
	}

	void GizmoWidget::RightMouseReleased(AbstractWidget* w)
	{
		GizmoWidget* self = reinterpret_cast<GizmoWidget*>(w);
		self->onRightMouseReleased();
	}

	void GizmoWidget::RightMousePressed(AbstractWidget* w)
	{
		GizmoWidget* self = reinterpret_cast<GizmoWidget*>(w);
		self->onRightMousePressed();
	}

	void GizmoWidget::MouseMove(AbstractWidget* w)
	{
		GizmoWidget* self = reinterpret_cast<GizmoWidget*>(w);
		self->onMouseMove();
	}
}