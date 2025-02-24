#include "GizmoObserver.h"
#include "CallbackCommand.h"
#include "RenderWindowInteractor.h"

namespace MOON {

	vtkInteractorObserver::vtkInteractorObserver()
	{
		this->Enabled = 0;

		this->Interactor = nullptr;

		this->EventCallbackCommand = CallbackCommand::New();
		this->EventCallbackCommand->SetClientData(this);

		// subclass has to invoke SetCallback()

		this->KeyPressCallbackCommand = CallbackCommand::New();
		this->KeyPressCallbackCommand->SetClientData(this);
		this->KeyPressCallbackCommand->SetCallback(vtkInteractorObserver::ProcessEvents);



		this->Priority = 0.0f;


		this->KeyPressActivation = 1;
		this->KeyPressActivationValue = 'i';

		this->CharObserverTag = 0;
		this->DeleteObserverTag = 0;
	}

	vtkInteractorObserver::~vtkInteractorObserver()
	{


		this->SetEnabled(0);
		this->SetInteractor(nullptr);
	}

	void vtkInteractorObserver::SetInteractor(vtkRenderWindowInteractor* i)
	{
		if (i == this->Interactor)
		{
			return;
		}



		// if we already have an Interactor then stop observing it
		if (this->Interactor)
		{
			this->SetEnabled(0); // disable the old interactor
			this->Interactor->RemoveObserver(this->CharObserverTag);
			this->CharObserverTag = 0;
			this->Interactor->RemoveObserver(this->DeleteObserverTag);
			this->DeleteObserverTag = 0;
		}

		this->Interactor = i;

		// add observers for each of the events handled in ProcessEvents
		if (i)
		{
			this->CharObserverTag =
				i->AddObserver(ExecuteCommand::CharEvent, this->KeyPressCallbackCommand, this->Priority);
			this->DeleteObserverTag =
				i->AddObserver(ExecuteCommand::DeleteEvent, this->KeyPressCallbackCommand, this->Priority);

		}
	}

	void vtkInteractorObserver::ProcessEvents(
		GizmoObject* object, unsigned long event, void* clientdata, void* calldata)
	{
		if (event == ExecuteCommand::CharEvent || event == ExecuteCommand::DeleteEvent)
		{
			GizmoObject* vobj = reinterpret_cast<GizmoObject*>(clientdata);
			vtkInteractorObserver* self = static_cast<vtkInteractorObserver*>(vobj);
			if (self)
			{
				if (event == ExecuteCommand::CharEvent)
				{
					self->OnChar();
				}
				else // delete event
				{
					self->SetInteractor(nullptr);
				}
			}
			else
			{

			}
		}
	}

	void vtkInteractorObserver::StartInteraction()
	{

	}

	//------------------------------------------------------------------------------
	void vtkInteractorObserver::EndInteraction()
	{

	}


	void vtkInteractorObserver::ComputeDisplayToWorld(double x, double y, double z, double worldPt[4])
	{

	}


	void vtkInteractorObserver::ComputeWorldToDisplay(double x, double y, double z, double displayPt[3])
	{

	}

	void vtkInteractorObserver::OnChar()
	{

		if (this->KeyPressActivation)
		{
			if (this->Interactor->GetKeyCode() == this->KeyPressActivationValue)
			{
				if (!this->Enabled)
				{
					this->On();
				}
				else
				{
					this->Off();
				}
				this->KeyPressCallbackCommand->SetAbortFlag(1);
			}
		}
	}

	void vtkInteractorObserver::GrabFocus(ExecuteCommand* mouseEvents, ExecuteCommand* keypressEvents)
	{
		if (this->Interactor)
		{
			this->Interactor->GrabFocus(mouseEvents, keypressEvents);
		}
	}

	void vtkInteractorObserver::ReleaseFocus()
	{
		if (this->Interactor)
		{
			this->Interactor->ReleaseFocus();
		}
	}
}
