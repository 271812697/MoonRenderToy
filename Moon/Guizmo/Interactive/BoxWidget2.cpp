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
	BoxWidget2::BoxWidget2()
	{
		this->WidgetState = BoxWidget2::Start;

		this->TranslationEnabled = true;
		this->ScalingEnabled = true;
		this->RotationEnabled = true;
		this->MoveFacesEnabled = true;

		// Define widget events
		this->CallbackMapper->SetCallbackMethod(ExecuteCommand::LeftButtonPressEvent, GizmoEvent::NoModifier, 0,
			0, nullptr, WidgetEvent::Select, this, BoxWidget2::SelectAction);
		this->CallbackMapper->SetCallbackMethod(ExecuteCommand::LeftButtonReleaseEvent, GizmoEvent::NoModifier,
			0, 0, nullptr, WidgetEvent::EndSelect, this, BoxWidget2::EndSelectAction);
		this->CallbackMapper->SetCallbackMethod(ExecuteCommand::MiddleButtonPressEvent,
			WidgetEvent::Translate, this, BoxWidget2::TranslateAction);
		this->CallbackMapper->SetCallbackMethod(ExecuteCommand::MiddleButtonReleaseEvent,
			WidgetEvent::EndTranslate, this, BoxWidget2::EndSelectAction);
		this->CallbackMapper->SetCallbackMethod(ExecuteCommand::LeftButtonPressEvent,
			GizmoEvent::ControlModifier, 0, 0, nullptr, WidgetEvent::Translate, this,
			BoxWidget2::TranslateAction);
		this->CallbackMapper->SetCallbackMethod(ExecuteCommand::LeftButtonReleaseEvent,
			GizmoEvent::ControlModifier, 0, 0, nullptr, WidgetEvent::EndTranslate, this,
			BoxWidget2::EndSelectAction);
		this->CallbackMapper->SetCallbackMethod(ExecuteCommand::LeftButtonPressEvent, GizmoEvent::ShiftModifier,
			0, 0, nullptr, WidgetEvent::Translate, this, BoxWidget2::TranslateAction);
		this->CallbackMapper->SetCallbackMethod(ExecuteCommand::LeftButtonReleaseEvent,
			GizmoEvent::ShiftModifier, 0, 0, nullptr, WidgetEvent::EndTranslate, this,
			BoxWidget2::EndSelectAction);
		this->CallbackMapper->SetCallbackMethod(
			ExecuteCommand::RightButtonPressEvent, WidgetEvent::Scale, this, BoxWidget2::ScaleAction);
		this->CallbackMapper->SetCallbackMethod(ExecuteCommand::RightButtonReleaseEvent,
			WidgetEvent::EndScale, this, BoxWidget2::EndSelectAction);
		this->CallbackMapper->SetCallbackMethod(
			ExecuteCommand::MouseMoveEvent, WidgetEvent::Move, this, BoxWidget2::MoveAction);

		this->KeyEventCallbackCommand = CallbackCommand::New();
		this->KeyEventCallbackCommand->SetClientData(this);
		this->KeyEventCallbackCommand->SetCallback(BoxWidget2::ProcessKeyEvents);
	}

	//------------------------------------------------------------------------------
	BoxWidget2::~BoxWidget2()
	{
		// this->KeyEventCallbackCommand->Delete();
	}

	//------------------------------------------------------------------------------
	void BoxWidget2::SetEnabled(int enabling)
	{
		int enabled = this->Enabled;

		// We do this step first because it sets the CurrentRenderer
		this->AbstractWidget::SetEnabled(enabling);

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
	void BoxWidget2::SelectAction(AbstractWidget* w)
	{
		// We are in a static method, cast to ourself
		BoxWidget2* self = reinterpret_cast<BoxWidget2*>(w);
	}

	//------------------------------------------------------------------------------
	void BoxWidget2::SelectAction3D(AbstractWidget* w)
	{
		BoxWidget2* self = reinterpret_cast<BoxWidget2*>(w);


	}

	//------------------------------------------------------------------------------
	void BoxWidget2::TranslateAction(AbstractWidget* w)
	{
		// We are in a static method, cast to ourself
		BoxWidget2* self = reinterpret_cast<BoxWidget2*>(w);
	}

	//------------------------------------------------------------------------------
	void BoxWidget2::ScaleAction(AbstractWidget* w)
	{
		// We are in a static method, cast to ourself
		BoxWidget2* self = reinterpret_cast<BoxWidget2*>(w);

	}

	//------------------------------------------------------------------------------
	void BoxWidget2::MoveAction(AbstractWidget* w)
	{
		BoxWidget2* self = reinterpret_cast<BoxWidget2*>(w);

		// See whether we're active
		if (self->WidgetState == BoxWidget2::Start)
		{
			return;
		}

	}

	//------------------------------------------------------------------------------
	void BoxWidget2::MoveAction3D(AbstractWidget* w)
	{
		BoxWidget2* self = reinterpret_cast<BoxWidget2*>(w);


	}

	//------------------------------------------------------------------------------
	void BoxWidget2::EndSelectAction(AbstractWidget* w)
	{
		BoxWidget2* self = reinterpret_cast<BoxWidget2*>(w);
		if (self->WidgetState == BoxWidget2::Start)
		{
			return;
		}

	}

	//------------------------------------------------------------------------------
	void BoxWidget2::EndSelectAction3D(AbstractWidget* w)
	{
		BoxWidget2* self = reinterpret_cast<BoxWidget2*>(w);

	}

	//------------------------------------------------------------------------------
	void BoxWidget2::StepAction3D(AbstractWidget* w)
	{
		BoxWidget2* self = reinterpret_cast<BoxWidget2*>(w);

	}

	BoxWidget2* BoxWidget2::New()
	{
		return new BoxWidget2();
	}

	//------------------------------------------------------------------------------
	void BoxWidget2::CreateDefaultRepresentation()
	{


	}

	//------------------------------------------------------------------------------
	void BoxWidget2::ProcessKeyEvents(GizmoObject*, unsigned long event, void* clientdata, void*)
	{
		BoxWidget2* self = static_cast<BoxWidget2*>(clientdata);

	}


}