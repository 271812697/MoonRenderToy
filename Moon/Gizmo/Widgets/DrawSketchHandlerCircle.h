#pragma once
#include "Gizmo/GizmoWidget.h"
#include "Gizmo/Widgets/DrawSketchDefaultHandler.h"

namespace MOON
{
	enum class CircleEllipseConstructionMethod
	{
		Center,
		ThreeRim,
		End  // Must be the last one
	};
	class DrawSketchHandlerCircle: public DrawSketchDefaultHandler<DrawSketchHandlerCircle, StateMachines::ThreeSeekEnd,3, CircleEllipseConstructionMethod>
	{
	public:
		DrawSketchHandlerCircle(const std::string& name);
		virtual ~DrawSketchHandlerCircle();
		virtual void onUpdate()override;
		virtual void onSetActive(bool flag)override;
		void onMouseClicked();
		void onMouseMove();
		void SetEnabled(int) override;
		virtual void updateDataAndDrawToPosition(Vec2 onSketchPos)override;
		virtual void onButtonPressed(Vec2 onSketchPos) override;
		bool canGoToNextMode() override;
		void createShape(bool onlyeditoutline) override;
	private:
		class DrawSketchHandlerCircleInternal;
		DrawSketchHandlerCircleInternal* m_internal = nullptr;
	};
}