#pragma once
#include "Interactive/EventWidget.h"
#include "Interactive/Widgets/DrawSketchDefaultHandler.h"
namespace MOON
{
	enum class ArcSlotConstructionMethod
	{
		ArcSlot,
		RectangleSlot,
		End  // Must be the last one
	};

	class DrawSketchHandlerArcSlot : public DrawSketchDefaultHandler<DrawSketchHandlerArcSlot, StateMachines::FourSeekEnd, 3, ArcSlotConstructionMethod>
	{
		using SupperClass = DrawSketchDefaultHandler<DrawSketchHandlerArcSlot, StateMachines::FourSeekEnd, 3, ArcSlotConstructionMethod>;
	public:
		DrawSketchHandlerArcSlot(const std::string& name);
		virtual ~DrawSketchHandlerArcSlot();
		virtual void onUpdate()override;
		virtual void updateDataAndDrawToPosition(Base::Vector2d onSketchPos)override;
		bool canGoToNextMode() override;
		void createShape(bool onlyeditoutline) override;
	private:
		void reverseIfNecessary();

		Base::Vector2d centerPoint, startPoint, endPoint;
		double startAngle, startAngleBackup, endAngle, arcAngle, r, radius;
		bool angleReversed;
	};
}
