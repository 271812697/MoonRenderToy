#pragma once
#include "Interactive/EventWidget.h"
namespace MOON
{
	class RotateCenter: public EventWidget
	{
	public:
		RotateCenter(const std::string& name);
		virtual ~RotateCenter();
		virtual void onUpdate()override;
		virtual void onLeftMousePressed()override;
		virtual void onLeftMouseReleased()override;
		virtual void onRightMousePressed()override;
		virtual void onRightMouseReleased()override;
		virtual void onMouseMove()override;
	};
}