#pragma once
#include "Interactive/EventWidget.h"
namespace MOON
{
	enum ArrowRotateEvent{
		AngleChange=1001,
		ArrowRotateEventNone
	};
	class ArrowRotateWidget: public EventWidget
	{
	public:
		ArrowRotateWidget(const std::string& name);
		virtual ~ArrowRotateWidget();
		virtual void onUpdate()override;
		void setUpRotateCenter(float x,float y,float z );
		void setUpRotateAxis(float x, float y, float z);
		void setUpOriginPos(float x, float y, float z);
		void setUpScale(float s);
		float getAngle();
		void setAngle(float len);
		
		enum IntersectiveState
		{
			Stop,
			Hot,
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