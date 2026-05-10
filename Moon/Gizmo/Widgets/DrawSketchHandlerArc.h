#pragma once
#include "Gizmo/GizmoWidget.h"
#include "Gizmo/Widgets/DrawSketchDefaultHandler.h"
#include "Gizmo/Widgets/CircleEllipseConstructionMethod.h"
namespace MOON
{

	class DrawSketchHandlerArc : public DrawSketchDefaultHandler<DrawSketchHandlerArc, StateMachines::ThreeSeekEnd,3, CircleEllipseConstructionMethod>
	{
	public:
		DrawSketchHandlerArc(const std::string& name);
		virtual ~DrawSketchHandlerArc();
		virtual void onUpdate()override;
		virtual void onSetActive(bool flag)override;

		virtual void updateDataAndDrawToPosition(Base::Vector2d onSketchPos)override;
		//virtual void onButtonPressed(Base::Vector2d onSketchPos) override;
		bool canGoToNextMode() override;
		void createShape(bool onlyeditoutline) override;
	private:
		class DrawSketchHandlerArcInternal;
		DrawSketchHandlerArcInternal* m_internal = nullptr;
	};
}