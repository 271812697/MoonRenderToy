#pragma once
#include "Interactive/EventWidget.h"
#include "Interactive/Widgets/DrawSketchDefaultHandler.h"
#include "Interactive/Widgets/CircleEllipseConstructionMethod.h"
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