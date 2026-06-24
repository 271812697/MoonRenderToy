#include "EventWidget.h"
#include "Interactive/Im3DRenderer.h"
#include "Interactive/Interactive/Event.h"
#include "Interactive/Interactive/ExecuteCommand.h"
#include "Interactive/Interactive/CallbackCommand.h"
#include "Interactive/Interactive/WidgetCallbackMapper.h"
#include "Interactive/WidgetEvent.h"
#include "Interactive/Interactive/WidgetEventTranslator.h"
#include "Interactive/Interactive/RenderWindowInteractor.h"
#include "Core/Global/ServiceLocator.h"
#include "renderer/SceneView.h"
namespace MOON {
	static unsigned int nextWidgetId = 0;
	EventWidget::EventWidget(const std::string& name) :mName(name), mWidgetId(++nextWidgetId)
	{
		ImRenderer::instance().addGizmoWidget(this);;
		renderer = &ImRenderer::instance();
		SetInteractor(RenderWindowInteractor::Instance());
		m_sceneView = &GetService(Editor::Panels::SceneView);

		// Define widget events
		this->CallbackMapper->SetCallbackMethod(ExecuteCommand::LeftButtonPressEvent, GizmoEvent::NoModifier, 0,
			0, 0, WidgetEvent::Select, this, EventWidget::LeftMousePressed);
		this->CallbackMapper->SetCallbackMethod(ExecuteCommand::LeftButtonReleaseEvent, GizmoEvent::NoModifier, 0,
			0, 0, WidgetEvent::Select3D, this, EventWidget::LeftMouseReleased);
		this->CallbackMapper->SetCallbackMethod(ExecuteCommand::RightButtonReleaseEvent, GizmoEvent::NoModifier, 0,
			0, 0, WidgetEvent::Completed, this, EventWidget::RightMouseReleased);
		this->CallbackMapper->SetCallbackMethod(ExecuteCommand::RightButtonPressEvent, GizmoEvent::NoModifier, 0,
			0, 0, WidgetEvent::EndSelect, this, EventWidget::RightMousePressed);
		this->CallbackMapper->SetCallbackMethod(ExecuteCommand::MouseMoveEvent, GizmoEvent::NoModifier, 0,
			0, 0, WidgetEvent::Move3D, this, EventWidget::MouseMove);

		this->KeyEventCallbackCommand = CallbackCommand::New();
		this->KeyEventCallbackCommand->SetClientData(this);
		this->KeyEventCallbackCommand->SetCallback(EventWidget::ProcessKeyEvents);
	}
	EventWidget::~EventWidget()
	{
		ImRenderer::instance().removeGizmoWidget(this);
		
		// don't listen for events any more
		if (!this->Parent)
		{
			this->Interactor->RemoveObserver(this->KeyEventCallbackCommand);
		}
		else
		{
			this->Parent->RemoveObserver(this->KeyEventCallbackCommand);
		}
		delete KeyEventCallbackCommand;
	}
	void EventWidget::setActive(bool flag)
	{
		 mActive = flag;
		 onSetActive(flag); 
		 SetEnabled(flag ? 1 : 0);
	}
	void EventWidget::setVisible(bool flag)
	{
		mVisible = flag;
	}
	void EventWidget::setImmediateInvoke(bool flag)
	{
		mImInvoke = flag;
	}
	void EventWidget::update()
	{
		if (mActive&&mVisible) {
			onUpdate();
		}
	}
	void EventWidget::onUpdate()
	{

	}
	void EventWidget::onSetActive(bool flag)
	{
	}
	void EventWidget::onLeftMousePressed()
	{
	}

	void EventWidget::onLeftMouseReleased()
	{
	}

	void EventWidget::onRightMousePressed()
	{
	}

	void EventWidget::onRightMouseReleased()
	{

	}

	void EventWidget::onMouseMove()
	{
	}

	void EventWidget::onKeyPress(const std::string& key)
	{
	}

	void EventWidget::onKeyRelease(const std::string& key)
	{
	}

	void EventWidget::SetEnabled(int enabling)
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

	void EventWidget::LeftMousePressed(AbstractWidget* w)
	{
		EventWidget* self = reinterpret_cast<EventWidget*>(w);
		self->onLeftMousePressed();
	}

	void EventWidget::LeftMouseReleased(AbstractWidget* w)
	{
		EventWidget* self = reinterpret_cast<EventWidget*>(w);
		self->onLeftMouseReleased();
	}

	void EventWidget::RightMouseReleased(AbstractWidget* w)
	{
		EventWidget* self = reinterpret_cast<EventWidget*>(w);
		self->onRightMouseReleased();
	}

	void EventWidget::RightMousePressed(AbstractWidget* w)
	{
		EventWidget* self = reinterpret_cast<EventWidget*>(w);
		self->onRightMousePressed();
	}

	void EventWidget::MouseMove(AbstractWidget* w)
	{
		EventWidget* self = reinterpret_cast<EventWidget*>(w);
		self->onMouseMove();
	}
	void EventWidget::ProcessKeyEvents(EventObject*, unsigned long event, void* clientdata, void*)
	{
		EventWidget* self = static_cast<EventWidget*>(clientdata);
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