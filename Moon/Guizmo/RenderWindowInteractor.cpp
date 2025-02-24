#include "RenderWindowInteractor.h"
#include "ExecuteCommand.h" 
#include <map>

namespace MOON {
bool vtkRenderWindowInteractor::InteractorManagesTheEventLoop = true;

vtkRenderWindowInteractor::vtkRenderWindowInteractor()
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
vtkRenderWindowInteractor::~vtkRenderWindowInteractor()
{

  delete[] this->KeySym;
 
}
//------------------------------------------------------------------------------
void vtkRenderWindowInteractor::Start()
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
void vtkRenderWindowInteractor::ExitCallback()
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
void vtkRenderWindowInteractor::UserCallback()
{
  this->InvokeEvent(ExecuteCommand::UserEvent, nullptr);
}

//------------------------------------------------------------------------------
void vtkRenderWindowInteractor::StartPickCallback()
{
  this->InvokeEvent(ExecuteCommand::StartPickEvent, nullptr);
}

//------------------------------------------------------------------------------
void vtkRenderWindowInteractor::EndPickCallback()
{
  this->InvokeEvent(ExecuteCommand::EndPickEvent, nullptr);
}
vtkRenderWindowInteractor* vtkRenderWindowInteractor::New()
{
    return new vtkRenderWindowInteractor();
}

//------------------------------------------------------------------------------
void vtkRenderWindowInteractor::Initialize()
{
  this->Initialized = 1;
  this->Enable();

}

void vtkRenderWindowInteractor::Terminate()
{
    delete this;
}


//------------------------------------------------------------------
void vtkRenderWindowInteractor::MouseMoveEvent()
{
  if (!this->Enabled)
  {
    return;
  }
  this->InvokeEvent(ExecuteCommand::MouseMoveEvent, nullptr);

}

//------------------------------------------------------------------
void vtkRenderWindowInteractor::RightButtonPressEvent()
{
  if (!this->Enabled)
  {
    return;
  }
  this->InvokeEvent(ExecuteCommand::RightButtonPressEvent, nullptr);
}

//------------------------------------------------------------------
void vtkRenderWindowInteractor::RightButtonReleaseEvent()
{
  if (!this->Enabled)
  {
    return;
  }
  this->InvokeEvent(ExecuteCommand::RightButtonReleaseEvent, nullptr);
}

//------------------------------------------------------------------
void vtkRenderWindowInteractor::LeftButtonPressEvent()
{
  if (!this->Enabled)
  {
    return;
  }


  this->InvokeEvent(ExecuteCommand::LeftButtonPressEvent, nullptr);
}

//------------------------------------------------------------------
void vtkRenderWindowInteractor::LeftButtonReleaseEvent()
{
  if (!this->Enabled)
  {
    return;
  }
  this->InvokeEvent(ExecuteCommand::LeftButtonReleaseEvent, nullptr);
}

//------------------------------------------------------------------
void vtkRenderWindowInteractor::MiddleButtonPressEvent()
{
  if (!this->Enabled)
  {
    return;
  }
  this->InvokeEvent(ExecuteCommand::MiddleButtonPressEvent, nullptr);
}

//------------------------------------------------------------------
void vtkRenderWindowInteractor::MiddleButtonReleaseEvent()
{
  if (!this->Enabled)
  {
    return;
  }
  this->InvokeEvent(ExecuteCommand::MiddleButtonReleaseEvent, nullptr);
}

//------------------------------------------------------------------
void vtkRenderWindowInteractor::MouseWheelForwardEvent()
{
  if (!this->Enabled)
  {
    return;
  }
  this->InvokeEvent(ExecuteCommand::MouseWheelForwardEvent, nullptr);
}

//------------------------------------------------------------------
void vtkRenderWindowInteractor::MouseWheelBackwardEvent()
{
  if (!this->Enabled)
  {
    return;
  }
  this->InvokeEvent(ExecuteCommand::MouseWheelBackwardEvent, nullptr);
}

//------------------------------------------------------------------
void vtkRenderWindowInteractor::MouseWheelLeftEvent()
{
  if (!this->Enabled)
  {
    return;
  }
  this->InvokeEvent(ExecuteCommand::MouseWheelLeftEvent, nullptr);
}

//------------------------------------------------------------------
void vtkRenderWindowInteractor::MouseWheelRightEvent()
{
  if (!this->Enabled)
  {
    return;
  }
  this->InvokeEvent(ExecuteCommand::MouseWheelRightEvent, nullptr);
}

void vtkRenderWindowInteractor::EnterEvent()
{
  if (!this->Enabled)
  {
    return;
  }
  this->InvokeEvent(ExecuteCommand::EnterEvent, nullptr);
}

//------------------------------------------------------------------
void vtkRenderWindowInteractor::LeaveEvent()
{
  if (!this->Enabled)
  {
    return;
  }
  this->InvokeEvent(ExecuteCommand::LeaveEvent, nullptr);
}

//------------------------------------------------------------------
void vtkRenderWindowInteractor::KeyPressEvent()
{
  if (!this->Enabled)
  {
    return;
  }
  this->InvokeEvent(ExecuteCommand::KeyPressEvent, nullptr);
}

//------------------------------------------------------------------
void vtkRenderWindowInteractor::KeyReleaseEvent()
{
  if (!this->Enabled)
  {
    return;
  }
  this->InvokeEvent(ExecuteCommand::KeyReleaseEvent, nullptr);
}
}
