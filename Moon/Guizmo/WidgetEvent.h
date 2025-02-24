#pragma once
#include "GizmoObject.h"
namespace MOON {

	class vtkWidgetEvent : public GizmoObject
	{
	public:
		static vtkWidgetEvent* New();
		enum WidgetEventIds
		{
			NoEvent = 0,
			Select,
			EndSelect,
			Delete,
			Translate,
			EndTranslate,
			Scale,
			EndScale,
			Resize,
			EndResize,
			Rotate,
			EndRotate,
			Move,
			SizeHandles,
			AddPoint,
			AddFinalPoint,
			Completed,
			PickPoint,
			PickNormal,
			PickDirectionPoint,
			TimedOut,
			ModifyEvent,
			Reset,
			Up,
			Down,
			Left,
			Right,
			Select3D,
			EndSelect3D,
			Move3D,
			AddPoint3D,
			AddFinalPoint3D,
			HoverLeave
		};

		static const char* GetStringFromEventId(unsigned long event);
		static unsigned long GetEventIdFromString(const char* event);

	protected:
		vtkWidgetEvent() = default;
		~vtkWidgetEvent() override = default;
	private:
		vtkWidgetEvent(const vtkWidgetEvent&) = delete;
		void operator=(const vtkWidgetEvent&) = delete;
	};

}