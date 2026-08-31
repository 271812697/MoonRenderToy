#pragma once
#include "Interactive/EventWidget.h"
#include "Interactive/Widgets/DrawSketchDefaultHandler.h"
namespace MOON
{
	class DrawSketchHandlerSlot : public DrawSketchDefaultHandler<DrawSketchHandlerSlot, StateMachines::ThreeSeekEnd, 2>
	{
		using SupperClass = DrawSketchDefaultHandler<DrawSketchHandlerSlot, StateMachines::ThreeSeekEnd, 2>;
	public:
		DrawSketchHandlerSlot(const std::string& name);
		virtual ~DrawSketchHandlerSlot();
		virtual void onUpdate()override;
		virtual void updateDataAndDrawToPosition(Base::Vector2d onSketchPos)override;
		bool canGoToNextMode() override;
		void createShape(bool onlyeditoutline) override;
		void onReset() override;
	private:
		Base::Vector2d startPoint, secondPoint;
		double radius, length, angle;
	};
}
