#pragma once
#include "Interactive/EventWidget.h"
namespace MOON
{
	enum AxisTranslationEvent{
		LengthChange=1001,
		None
	};
	class AxisTranslationWidget: public EventWidget
	{
	public:
		AxisTranslationWidget(const std::string& name);
		virtual ~AxisTranslationWidget();
		virtual void onUpdate()override;
		void setUpOrigin(float x,float y,float z );
		void setUpDir(float x, float y, float z);
		void setUpScale(float s);
		float getLength();
		void setLength(float len);
		
		enum IntersectiveState
		{
			Stop,
			Hot,
			AxisT
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