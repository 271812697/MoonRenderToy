#include "GizmoWidget.h"
#include "Gizmo/Gizmo.h"
#include "Gizmo/Interactive/Event.h"
#include "Gizmo/Interactive/ExecuteCommand.h"
#include "Gizmo/Interactive/CallbackCommand.h"
#include "Gizmo/Interactive/WidgetCallbackMapper.h"
#include "Gizmo/Interactive/WidgetEvent.h"
#include "Gizmo/Interactive/WidgetEventTranslator.h"
#include "Gizmo/Interactive/RenderWindowInteractor.h"
#include "Core/Global/ServiceLocator.h"
#include "renderer/SceneView.h"
namespace MOON {
	static unsigned int nextWidgetId = 0;
	GizmoWidget::GizmoWidget(const std::string& name) :mName(name), mWidgetId(++nextWidgetId)
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

		this->KeyEventCallbackCommand = CallbackCommand::New();
		this->KeyEventCallbackCommand->SetClientData(this);
		this->KeyEventCallbackCommand->SetCallback(GizmoWidget::ProcessKeyEvents);
	}
	GizmoWidget::~GizmoWidget()
	{
		Gizmo::instance().removeGizmoWidget(this);
		delete KeyEventCallbackCommand;
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

	void GizmoWidget::onKeyPress(const std::string& key)
	{
	}

	void GizmoWidget::onKeyRelease(const std::string& key)
	{
	}

	void GizmoWidget::SetEnabled(int enabling)
	{
		int enabled = this->Enabled;
		// We do this step first because it sets the CurrentRenderer
		AbstractWidget::SetEnabled(enabling);

		// We defer enabling the handles until the selection process begins
		if (enabling && !enabled)
		{
			this->Interactor->AddObserver(
				ExecuteCommand::KeyPressEvent, this->KeyEventCallbackCommand, this->Priority);
			this->Interactor->AddObserver(
				ExecuteCommand::KeyReleaseEvent, this->KeyEventCallbackCommand, this->Priority);
		}
		else if (!enabling && enabled)
		{
			this->Interactor->RemoveObserver(this->KeyEventCallbackCommand);
		}
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
	void GizmoWidget::ProcessKeyEvents(GizmoObject*, unsigned long event, void* clientdata, void*)
	{
		GizmoWidget* self = static_cast<GizmoWidget*>(clientdata);
		char* cKeySym = self->Interactor->GetKeySym();
		std::string keySym = cKeySym != nullptr ? cKeySym : "";
		std::transform(keySym.begin(), keySym.end(), keySym.begin(), ::toupper);
		if (event == ExecuteCommand::KeyPressEvent)
		{
			self->onKeyPress(keySym);
		}
		else if (event == ExecuteCommand::KeyReleaseEvent)
		{
			self->onKeyRelease(keySym);
		}
	}
}