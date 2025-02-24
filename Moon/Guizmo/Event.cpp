#include "Event.h"
#include "ExecuteCommand.h"
#include "RenderWindowInteractor.h"

namespace MOON {
	vtkEvent::vtkEvent()
	{
		this->Modifier = vtkEvent::AnyModifier;
		this->KeyCode = 0;
		this->RepeatCount = 0;
		this->KeySym = nullptr;
		this->EventId = ExecuteCommand::NoEvent;
	}

	vtkEvent::~vtkEvent()
	{
		delete[] this->KeySym;
	}

	bool vtkEvent::operator==(unsigned long VTKEvent) const
	{
		return this->EventId == VTKEvent;
	}

	bool vtkEvent::operator==(vtkEvent* e) const
	{
		if (this->EventId != e->EventId)
		{
			return false;
		}
		if (this->Modifier != vtkEvent::AnyModifier && e->Modifier != vtkEvent::AnyModifier &&
			this->Modifier != e->Modifier)
		{
			return false;
		}
		if (this->KeyCode != '\0' && e->KeyCode != '\0' && this->KeyCode != e->KeyCode)
		{
			return false;
		}
		if (this->RepeatCount != 0 && e->RepeatCount != 0 && this->RepeatCount != e->RepeatCount)
		{
			return false;
		}
		if (this->KeySym != nullptr && e->KeySym != nullptr && strcmp(this->KeySym, e->KeySym) != 0)
		{
			return false;
		}

		return true;
	}

	vtkEvent* vtkEvent::New()
	{
		auto result = new vtkEvent;
		return result;
	}

	int vtkEvent::GetModifier(vtkRenderWindowInteractor* i)
	{
		int modifier = 0;
		modifier |= (i->GetShiftKey() ? vtkEvent::ShiftModifier : 0);
		modifier |= (i->GetControlKey() ? vtkEvent::ControlModifier : 0);
		modifier |= (i->GetAltKey() ? vtkEvent::AltModifier : 0);

		return modifier;
	}


}
