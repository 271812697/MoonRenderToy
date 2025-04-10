#include "RenderWindowInteractor.h"
#include "ExecuteCommand.h" 
#include <map>

namespace MOON {
bool RenderWindowInteractor::InteractorManagesTheEventLoop = true;

RenderWindowInteractor::RenderWindowInteractor()
{
 
  this->Initialized = 0;
  this->Enabled = 0;
  this->EnableRender = true;
  this->DesiredUpdateRate = 15;

  this->StillUpdateRate = 0.0001;


  this->EventPosition[0] = this->LastEventPosition[0] = 0;
  this->EventPosition[1] = this->LastEventPosition[1] = 0;



  this->EventSize[0] = 0;
  this->EventSize[1] = 0;

  this->Size[0] = 0;
  this->Size[1] = 0;

  this->Translation[0] = this->LastTranslation[0] = 0;
  this->Translation[1] = this->LastTranslation[1] = 0;


  this->AltKey = 0;
  this->ControlKey = 0;
  this->ShiftKey = 0;
  this->KeyCode = 0;
  this->Rotation = 0;
  this->LastRotation = 0;
  this->Scale = 0;
  this->LastScale = 0;
  this->RepeatCount = 0;
  this->KeySym = nullptr;
  this->HandleEventLoop = false;
  this->Done = false;
}

//------------------------------------------------------------------------------
RenderWindowInteractor::~RenderWindowInteractor()
{

  delete[] this->KeySym;
 
}
//------------------------------------------------------------------------------
void RenderWindowInteractor::Start()
{

  if (this->HasObserver(ExecuteCommand::StartEvent) && !this->HandleEventLoop)
  {
    this->InvokeEvent(ExecuteCommand::StartEvent, nullptr);
    return;
  }

  if (!this->Initialized)
  {
    this->Initialize();

    if (!this->Initialized)
    {
      return;
    }
  }

  this->Done = false;
  this->StartEventLoop();
}




//------------------------------------------------------------------------------
void RenderWindowInteractor::ExitCallback()
{
  if (this->HasObserver(ExecuteCommand::ExitEvent))
  {
    this->InvokeEvent(ExecuteCommand::ExitEvent, nullptr);
  }
  else
  {
    this->TerminateApp();
  }
}

//------------------------------------------------------------------------------
void RenderWindowInteractor::UserCallback()
{
  this->InvokeEvent(ExecuteCommand::UserEvent, nullptr);
}

//------------------------------------------------------------------------------
void RenderWindowInteractor::StartPickCallback()
{
  this->InvokeEvent(ExecuteCommand::StartPickEvent, nullptr);
}

//------------------------------------------------------------------------------
void RenderWindowInteractor::EndPickCallback()
{
  this->InvokeEvent(ExecuteCommand::EndPickEvent, nullptr);
}
RenderWindowInteractor* RenderWindowInteractor::New()
{
    return new RenderWindowInteractor();
}

//------------------------------------------------------------------------------
void RenderWindowInteractor::Initialize()
{
  this->Initialized = 1;
  this->Enable();

}

void RenderWindowInteractor::Terminate()
{
    delete this;
}


//------------------------------------------------------------------
void RenderWindowInteractor::MouseMoveEvent()
{
  if (!this->Enabled)
  {
    return;
  }
  this->InvokeEvent(ExecuteCommand::MouseMoveEvent, nullptr);

}

//------------------------------------------------------------------
void RenderWindowInteractor::RightButtonPressEvent()
{
  if (!this->Enabled)
  {
    return;
  }
  this->InvokeEvent(ExecuteCommand::RightButtonPressEvent, nullptr);
}

//------------------------------------------------------------------
void RenderWindowInteractor::RightButtonReleaseEvent()
{
  if (!this->Enabled)
  {
    return;
  }
  this->InvokeEvent(ExecuteCommand::RightButtonReleaseEvent, nullptr);
}

//------------------------------------------------------------------
void RenderWindowInteractor::LeftButtonPressEvent()
{
  if (!this->Enabled)
  {
    return;
  }


  this->InvokeEvent(ExecuteCommand::LeftButtonPressEvent, nullptr);
}

//------------------------------------------------------------------
void RenderWindowInteractor::LeftButtonReleaseEvent()
{
  if (!this->Enabled)
  {
    return;
  }
  this->InvokeEvent(ExecuteCommand::LeftButtonReleaseEvent, nullptr);
}

//------------------------------------------------------------------
void RenderWindowInteractor::MiddleButtonPressEvent()
{
  if (!this->Enabled)
  {
    return;
  }
  this->InvokeEvent(ExecuteCommand::MiddleButtonPressEvent, nullptr);
}

//------------------------------------------------------------------
void RenderWindowInteractor::MiddleButtonReleaseEvent()
{
  if (!this->Enabled)
  {
    return;
  }
  this->InvokeEvent(ExecuteCommand::MiddleButtonReleaseEvent, nullptr);
}

//------------------------------------------------------------------
void RenderWindowInteractor::MouseWheelForwardEvent()
{
  if (!this->Enabled)
  {
    return;
  }
  this->InvokeEvent(ExecuteCommand::MouseWheelForwardEvent, nullptr);
}

//------------------------------------------------------------------
void RenderWindowInteractor::MouseWheelBackwardEvent()
{
  if (!this->Enabled)
  {
    return;
  }
  this->InvokeEvent(ExecuteCommand::MouseWheelBackwardEvent, nullptr);
}

//------------------------------------------------------------------
void RenderWindowInteractor::MouseWheelLeftEvent()
{
  if (!this->Enabled)
  {
    return;
  }
  this->InvokeEvent(ExecuteCommand::MouseWheelLeftEvent, nullptr);
}

//------------------------------------------------------------------
void RenderWindowInteractor::MouseWheelRightEvent()
{
  if (!this->Enabled)
  {
    return;
  }
  this->InvokeEvent(ExecuteCommand::MouseWheelRightEvent, nullptr);
}

void RenderWindowInteractor::EnterEvent()
{
  if (!this->Enabled)
  {
    return;
  }
  this->InvokeEvent(ExecuteCommand::EnterEvent, nullptr);
}

//------------------------------------------------------------------
void RenderWindowInteractor::LeaveEvent()
{
  if (!this->Enabled)
  {
    return;
  }
  this->InvokeEvent(ExecuteCommand::LeaveEvent, nullptr);
}

//------------------------------------------------------------------
void RenderWindowInteractor::KeyPressEvent()
{
  if (!this->Enabled)
  {
    return;
  }
  this->InvokeEvent(ExecuteCommand::KeyPressEvent, nullptr);
}

//------------------------------------------------------------------
void RenderWindowInteractor::KeyReleaseEvent()
{
  if (!this->Enabled)
  {
    return;
  }
  this->InvokeEvent(ExecuteCommand::KeyReleaseEvent, nullptr);
}
}
