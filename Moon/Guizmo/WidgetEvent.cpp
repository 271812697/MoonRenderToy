#include "WidgetEvent.h"
#include <algorithm>
namespace MOON {

	static const char* vtkWidgetEventStrings[] = {
		  "NoEvent",
		  "Select",
		  "EndSelect",
		  "Delete",
		  "Translate",
		  "EndTranslate",
		  "Scale",
		  "EndScale",
		  "Resize",
		  "EndResize",
		  "Rotate",
		  "EndRotate",
		  "Move",
		  "SizeHandles",
		  "AddPoint",
		  "AddFinalPoint",
		  "Completed",
		  "TimedOut",
		  "ModifyEvent",
		  "Reset",
		  "HoverLeave",
		  nullptr,
	};
	vtkWidgetEvent* vtkWidgetEvent::New()
	{
		auto result = new vtkWidgetEvent;
		return result;
	}

	//------------------------------------------------------------------------------
	const char* vtkWidgetEvent::GetStringFromEventId(unsigned long event)
	{
		static unsigned long numevents = 0;

		// find length of table
		if (!numevents)
		{
			while (vtkWidgetEventStrings[numevents] != nullptr)
			{
				numevents++;
			}
		}

		if (event < numevents)
		{
			return vtkWidgetEventStrings[event];
		}
		else
		{
			return "NoEvent";
		}
	}

	//------------------------------------------------------------------------------
	unsigned long vtkWidgetEvent::GetEventIdFromString(const char* event)
	{
		unsigned long i;

		for (i = 0; vtkWidgetEventStrings[i] != nullptr; i++)
		{
			if (!strcmp(vtkWidgetEventStrings[i], event))
			{
				return i;
			}
		}
		return vtkWidgetEvent::NoEvent;
	}

}