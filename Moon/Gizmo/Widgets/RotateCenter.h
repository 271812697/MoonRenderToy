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
		void onMouseLeftButtonPressed();
		void onMouseLeftButtonReleased();
		void onMouseMove();
	private:
		ExecuteCommandPair m_rightButtonPressObserver;
		ExecuteCommandPair m_rightButtonReleaseObserver;
		ExecuteCommandPair m_leftButtonPressObserver;
		ExecuteCommandPair m_leftButtonReleaseObserver;
		ExecuteCommandPair m_mouseMoveObserver;
	};
}