#pragma once
#include "Interactive/EventWidget.h"
#include "Interactive/Widgets/DrawSketchDefaultHandler.h"
namespace MOON
{
	enum class LineConstructionMethod
	{
		OnePointLengthAngle,
		OnePointWidthHeight,
		TwoPoints,
		End  // Must be the last one
	};
	class DrawSketchHandlerLine: public DrawSketchDefaultHandler<DrawSketchHandlerLine, StateMachines::TwoSeekEnd,2, LineConstructionMethod>
	{
		using SupperClass = DrawSketchDefaultHandler<DrawSketchHandlerLine, StateMachines::TwoSeekEnd, 2, LineConstructionMethod>;
	public:
		DrawSketchHandlerLine(const std::string& name, LineConstructionMethod constrMethod=LineConstructionMethod::OnePointLengthAngle );
		virtual ~DrawSketchHandlerLine();
		virtual void onUpdate()override;
		virtual void updateDataAndDrawToPosition(Base::Vector2d onSketchPos)override;
		void createShape(bool onlyeditoutline) override;
		void onReset() override;
	private:
		class Internal;
		Internal* m_internal = nullptr;
		Base::Vector2d startPoint, endPoint;
		double length;

		// These store the direction sign when OVP is first set to prevent sign flipping
		int lengthSign, widthSign;
		// Direction tracking to check once OVP is locked
		Base::Vector2d capturedDirection;
	};
}