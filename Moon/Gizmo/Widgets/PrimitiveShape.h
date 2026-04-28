#pragma once
#include "Gizmo/GizmoWidget.h"
#include <Eigen/Core>

namespace MOON
{
	class PrimitiveShape : public GizmoWidget
	{
	public:
		PrimitiveShape(const std::string& name);
		virtual ~PrimitiveShape()override;
		virtual void onLeftMousePressed();
		virtual void onLeftMouseReleased();
		virtual void onRightMousePressed();
		virtual void onRightMouseReleased();
		virtual void onMouseMove();
		virtual void createTopoShape();
		static void LeftMousePressed(AbstractWidget*);
		static void LeftMouseReleased(AbstractWidget*);
		static void RightMouseReleased(AbstractWidget*);
		static void RightMousePressed(AbstractWidget*);
		static void MouseMove(AbstractWidget*);
	};
}