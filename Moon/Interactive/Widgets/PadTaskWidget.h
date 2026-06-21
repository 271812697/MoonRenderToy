#pragma once
#include "Interactive/EventWidget.h"
namespace MOON
{
	enum PadTaskEvent{
		LengthChange=1001,
		AngleChange,
		None
	};
	class PadTaskWidget: public EventWidget
	{
	public:
		PadTaskWidget(const std::string& name);
		virtual ~PadTaskWidget();
		virtual void onUpdate()override;
		void setUpOrigin(float x,float y,float z );
		void setUpDir(float x, float y, float z);
		void setUpXAxis(float x,float y,float z);
		void setUpYAxis(float x, float y, float z);
		float getLength();
		float getAngle();
		void setLength(float len);
		void setAngle(float degree);
		enum IntersectiveState
		{
			Stop,
			Hot,
			AxisT,
			Rotate
		};
		virtual void onLeftMousePressed()override;
		virtual void onLeftMouseReleased()override;
		virtual void onMouseMove()override;
	private:
		class Internal;
		Internal* mInternal = nullptr;
		IntersectiveState mState;
	};
}