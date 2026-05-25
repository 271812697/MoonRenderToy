#pragma once
#include "Gizmo/GizmoWidget.h"
#include "Gizmo/Widgets/DrawSketchDefaultHandler.h"
#include "Gizmo/Widgets/CircleEllipseConstructionMethod.h"
namespace MOON
{
	class DrawSketchHandlerCircle: public DrawSketchDefaultHandler<DrawSketchHandlerCircle, StateMachines::ThreeSeekEnd,3, CircleEllipseConstructionMethod>
	{
	public:
		DrawSketchHandlerCircle(const std::string& name);
		virtual ~DrawSketchHandlerCircle();
		virtual void onUpdate()override;
		virtual void onSetActive(bool flag)override;

		virtual void updateDataAndDrawToPosition(Base::Vector2d onSketchPos)override;
		virtual void onButtonPressed(Base::Vector2d onSketchPos) override;
		bool canGoToNextMode() override;
		void createShape(bool onlyeditoutline) override;
	private:
		class DrawSketchHandlerCircleInternal;
		DrawSketchHandlerCircleInternal* m_internal = nullptr;
	};
}