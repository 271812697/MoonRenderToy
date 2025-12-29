#pragma once
#include "Gizmo/GizmoWidget.h"
namespace MOON
{
	class RotateCenter: public GizmoWidget
	{
	public:
		RotateCenter(const std::string& name);
		virtual ~RotateCenter();
		virtual void onUpdate()override;
		void onMouseRightButtonPressed();
		void onMouseRightButtonReleased();
		void onMouseMove();
	private:
		ExecuteCommandPair m_rightButtonPressObserver;
		ExecuteCommandPair m_rightButtonReleaseObserver;
		ExecuteCommandPair m_mouseMoveObserver;
	};
}