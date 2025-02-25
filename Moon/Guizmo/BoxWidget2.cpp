#include "BoxWidget2.h"
#include "CallbackCommand.h"
#include "ExecuteCommand.h"
#include "Event.h"
#include "EventData.h"

#include "RenderWindowInteractor.h"
#include "WidgetCallbackMapper.h"
#include "WidgetEvent.h"
#include "WidgetEventTranslator.h"
#include <algorithm>

namespace MOON {

	//------------------------------------------------------------------------------
	vtkBoxWidget2::vtkBoxWidget2()
	{
		this->WidgetState = vtkBoxWidget2::Start;

		this->TranslationEnabled = true;
		this->ScalingEnabled = true;
		this->RotationEnabled = true;
		this->MoveFacesEnabled = true;

		// Define widget events
		this->CallbackMapper->SetCallbackMethod(ExecuteCommand::LeftButtonPressEvent, vtkEvent::NoModifier, 0,
			0, nullptr, vtkWidgetEvent::Select, this, vtkBoxWidget2::SelectAction);
		this->CallbackMapper->SetCallbackMethod(ExecuteCommand::LeftButtonReleaseEvent, vtkEvent::NoModifier,
			0, 0, nullptr, vtkWidgetEvent::EndSelect, this, vtkBoxWidget2::EndSelectAction);
		this->CallbackMapper->SetCallbackMethod(ExecuteCommand::MiddleButtonPressEvent,
			vtkWidgetEvent::Translate, this, vtkBoxWidget2::TranslateAction);
		this->CallbackMapper->SetCallbackMethod(ExecuteCommand::MiddleButtonReleaseEvent,
			vtkWidgetEvent::EndTranslate, this, vtkBoxWidget2::EndSelectAction);
		this->CallbackMapper->SetCallbackMethod(ExecuteCommand::LeftButtonPressEvent,
			vtkEvent::ControlModifier, 0, 0, nullptr, vtkWidgetEvent::Translate, this,
			vtkBoxWidget2::TranslateAction);
		this->CallbackMapper->SetCallbackMethod(ExecuteCommand::LeftButtonReleaseEvent,
			vtkEvent::ControlModifier, 0, 0, nullptr, vtkWidgetEvent::EndTranslate, this,
			vtkBoxWidget2::EndSelectAction);
		this->CallbackMapper->SetCallbackMethod(ExecuteCommand::LeftButtonPressEvent, vtkEvent::ShiftModifier,
			0, 0, nullptr, vtkWidgetEvent::Translate, this, vtkBoxWidget2::TranslateAction);
		this->CallbackMapper->SetCallbackMethod(ExecuteCommand::LeftButtonReleaseEvent,
			vtkEvent::ShiftModifier, 0, 0, nullptr, vtkWidgetEvent::EndTranslate, this,
			vtkBoxWidget2::EndSelectAction);
		this->CallbackMapper->SetCallbackMethod(
			ExecuteCommand::RightButtonPressEvent, vtkWidgetEvent::Scale, this, vtkBoxWidget2::ScaleAction);
		this->CallbackMapper->SetCallbackMethod(ExecuteCommand::RightButtonReleaseEvent,
			vtkWidgetEvent::EndScale, this, vtkBoxWidget2::EndSelectAction);
		this->CallbackMapper->SetCallbackMethod(
			ExecuteCommand::MouseMoveEvent, vtkWidgetEvent::Move, this, vtkBoxWidget2::MoveAction);

		this->KeyEventCallbackCommand = CallbackCommand::New();
		this->KeyEventCallbackCommand->SetClientData(this);
		this->KeyEventCallbackCommand->SetCallback(vtkBoxWidget2::ProcessKeyEvents);
	}

	//------------------------------------------------------------------------------
	vtkBoxWidget2::~vtkBoxWidget2()
	{
		// this->KeyEventCallbackCommand->Delete();
	}

	//------------------------------------------------------------------------------
	void vtkBoxWidget2::SetEnabled(int enabling)
	{
		int enabled = this->Enabled;

		// We do this step first because it sets the CurrentRenderer
		this->vtkAbstractWidget::SetEnabled(enabling);

		// We defer enabling the handles until the selection process begins
		if (enabling && !enabled)
		{
			if (this->Parent)
			{
				this->Parent->AddObserver(
					ExecuteCommand::KeyPressEvent, this->KeyEventCallbackCommand, this->Priority);
				this->Parent->AddObserver(
					ExecuteCommand::KeyReleaseEvent, this->KeyEventCallbackCommand, this->Priority);
			}
			else
			{
				this->Interactor->AddObserver(
					ExecuteCommand::KeyPressEvent, this->KeyEventCallbackCommand, this->Priority);
				this->Interactor->AddObserver(
					ExecuteCommand::KeyReleaseEvent, this->KeyEventCallbackCommand, this->Priority);
			}
		}
		else if (!enabling && enabled)
		{
			if (this->Parent)
			{
				this->Parent->RemoveObserver(this->KeyEventCallbackCommand);
			}
			else
			{
				this->Interactor->RemoveObserver(this->KeyEventCallbackCommand);
			}
		}
	}

	//------------------------------------------------------------------------------
	void vtkBoxWidget2::SelectAction(vtkAbstractWidget* w)
	{
		// We are in a static method, cast to ourself
		vtkBoxWidget2* self = reinterpret_cast<vtkBoxWidget2*>(w);
	}

	//------------------------------------------------------------------------------
	void vtkBoxWidget2::SelectAction3D(vtkAbstractWidget* w)
	{
		vtkBoxWidget2* self = reinterpret_cast<vtkBoxWidget2*>(w);


	}

	//------------------------------------------------------------------------------
	void vtkBoxWidget2::TranslateAction(vtkAbstractWidget* w)
	{
		// We are in a static method, cast to ourself
		vtkBoxWidget2* self = reinterpret_cast<vtkBoxWidget2*>(w);
	}

	//------------------------------------------------------------------------------
	void vtkBoxWidget2::ScaleAction(vtkAbstractWidget* w)
	{
		// We are in a static method, cast to ourself
		vtkBoxWidget2* self = reinterpret_cast<vtkBoxWidget2*>(w);

	}

	//------------------------------------------------------------------------------
	void vtkBoxWidget2::MoveAction(vtkAbstractWidget* w)
	{
		vtkBoxWidget2* self = reinterpret_cast<vtkBoxWidget2*>(w);

		// See whether we're active
		if (self->WidgetState == vtkBoxWidget2::Start)
		{
			return;
		}

	}

	//------------------------------------------------------------------------------
	void vtkBoxWidget2::MoveAction3D(vtkAbstractWidget* w)
	{
		vtkBoxWidget2* self = reinterpret_cast<vtkBoxWidget2*>(w);


	}

	//------------------------------------------------------------------------------
	void vtkBoxWidget2::EndSelectAction(vtkAbstractWidget* w)
	{
		vtkBoxWidget2* self = reinterpret_cast<vtkBoxWidget2*>(w);
		if (self->WidgetState == vtkBoxWidget2::Start)
		{
			return;
		}

	}

	//------------------------------------------------------------------------------
	void vtkBoxWidget2::EndSelectAction3D(vtkAbstractWidget* w)
	{
		vtkBoxWidget2* self = reinterpret_cast<vtkBoxWidget2*>(w);

	}

	//------------------------------------------------------------------------------
	void vtkBoxWidget2::StepAction3D(vtkAbstractWidget* w)
	{
		vtkBoxWidget2* self = reinterpret_cast<vtkBoxWidget2*>(w);

	}

	vtkBoxWidget2* vtkBoxWidget2::New()
	{
		return new vtkBoxWidget2();
	}

	//------------------------------------------------------------------------------
	void vtkBoxWidget2::CreateDefaultRepresentation()
	{


	}

	//------------------------------------------------------------------------------
	void vtkBoxWidget2::ProcessKeyEvents(GizmoObject*, unsigned long event, void* clientdata, void*)
	{
		vtkBoxWidget2* self = static_cast<vtkBoxWidget2*>(clientdata);

	}


}