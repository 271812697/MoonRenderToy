#include "AbstractWidget.h"
#include "CallbackCommand.h"
#include "Event.h"
#include "RenderWindowInteractor.h"
#include "WidgetCallbackMapper.h"
#include "WidgetEvent.h"
#include "WidgetEventTranslator.h"
//------------------------------------------------------------------------------
namespace MOON {
	vtkAbstractWidget::vtkAbstractWidget()
	{
		// Setup event processing
		this->EventCallbackCommand->SetCallback(vtkAbstractWidget::ProcessEventsHandler);
		// There is no parent to this widget currently
		this->Parent = nullptr;
		// Set priority higher than interactor styles
		this->Priority = 0.5;


		// Does this widget respond to interaction?
		this->ProcessEvents = 1;
		// Okay, set up the event translations for the subclasses.
		this->EventTranslator = vtkWidgetEventTranslator::New();
		this->CallbackMapper = vtkWidgetCallbackMapper::New();
		this->CallbackMapper->SetEventTranslator(this->EventTranslator);
	}

	vtkAbstractWidget::~vtkAbstractWidget()
	{
		this->SetEnabled(0);
	}

	void vtkAbstractWidget::SetEnabled(int enabling)
	{
		if (enabling) //----------------
		{

			if (this->Enabled) // already enabled, just return
			{
				return;
			}

			if (!this->Interactor)
			{

				return;
			}

			// We're ready to enable
			this->Enabled = 1;

			// listen for the events found in the EventTranslator
			if (!this->Parent)
			{
				this->EventTranslator->AddEventsToInteractor(
					this->Interactor, this->EventCallbackCommand, this->Priority);
			}
			else
			{
				this->EventTranslator->AddEventsToParent(
					this->Parent, this->EventCallbackCommand, this->Priority);
			}


			//this->WidgetRep->BuildRepresentation();


			this->InvokeEvent(ExecuteCommand::EnableEvent, nullptr);
		}

		else // disabling------------------
		{
			if (!this->Enabled) // already disabled, just return
			{
				return;
			}

			this->Enabled = 0;

			// don't listen for events any more
			if (!this->Parent)
			{
				this->Interactor->RemoveObserver(this->EventCallbackCommand);
			}
			else
			{
				this->Parent->RemoveObserver(this->EventCallbackCommand);
			}
			this->InvokeEvent(ExecuteCommand::DisableEvent, nullptr);
		}
	}

	//------------------------------------------------------------------------------
	void vtkAbstractWidget::ProcessEventsHandler(
		GizmoObject* object, unsigned long vtkEvent, void* clientdata, void* calldata)
	{
		vtkAbstractWidget* self = reinterpret_cast<vtkAbstractWidget*>(clientdata);

		// if ProcessEvents is Off, we ignore all interaction events.
		if (!self->GetProcessEvents())
		{
			return;
		}

		// if the event has data then get the translation using the
		// event data
		unsigned long widgetEvent = vtkWidgetEvent::NoEvent;
		if (calldata && ExecuteCommand::EventHasData(vtkEvent))
		{
			widgetEvent =
				self->EventTranslator->GetTranslation(vtkEvent, static_cast<vtkEventData*>(calldata));
		}
		else
		{
			int modifier = vtkEvent::GetModifier(self->Interactor);

			// If neither the ctrl nor the shift keys are pressed, give
			// NoModifier a preference over AnyModifier.
			if (modifier == vtkEvent::AnyModifier)
			{
				widgetEvent = self->EventTranslator->GetTranslation(vtkEvent, vtkEvent::NoModifier,
					self->Interactor->GetKeyCode(), self->Interactor->GetRepeatCount(),
					self->Interactor->GetKeySym());
			}

			if (widgetEvent == vtkWidgetEvent::NoEvent)
			{
				widgetEvent =
					self->EventTranslator->GetTranslation(vtkEvent, modifier, self->Interactor->GetKeyCode(),
						self->Interactor->GetRepeatCount(), self->Interactor->GetKeySym());
			}
		}

		// Save the call data for widgets if needed
		self->CallData = calldata;

		// Invoke the widget callback
		if (widgetEvent != vtkWidgetEvent::NoEvent)
		{
			self->CallbackMapper->InvokeCallback(widgetEvent);
		}
	}

	//------------------------------------------------------------------------------
	void vtkAbstractWidget::Render()
	{
		if (!this->Parent && this->Interactor)
		{

		}
	}

	//------------------------------------------------------------------------------
	void vtkAbstractWidget::SetPriority(float f)
	{
		if (f != this->Priority)
		{
			this->vtkInteractorObserver::SetPriority(f);

			if (this->Enabled)
			{
				if (this->Interactor)
				{
					this->Interactor->RemoveObserver(this->CharObserverTag);
					this->Interactor->RemoveObserver(this->DeleteObserverTag);
					this->CharObserverTag = this->Interactor->AddObserver(
						ExecuteCommand::CharEvent, this->KeyPressCallbackCommand, this->Priority);
					this->DeleteObserverTag = this->Interactor->AddObserver(
						ExecuteCommand::DeleteEvent, this->KeyPressCallbackCommand, this->Priority);
				}

				if (!this->Parent)
				{
					if (this->Interactor)
					{
						this->Interactor->RemoveObserver(this->EventCallbackCommand);
					}
				}
				else
				{
					this->Parent->RemoveObserver(this->EventCallbackCommand);
				}

				if (!this->Parent)
				{
					if (this->Interactor)
					{
						this->EventTranslator->AddEventsToInteractor(
							this->Interactor, this->EventCallbackCommand, this->Priority);
					}
				}
				else
				{
					this->EventTranslator->AddEventsToParent(
						this->Parent, this->EventCallbackCommand, this->Priority);
				}
			}
		}
	}
}