#pragma once
#include "Interactive/EventWidget.h"
#include "Interactive/Widgets/DrawSketchDefaultHandler.h"
namespace MOON
{
	class DrawSketchHandlerPolygon : public DrawSketchDefaultHandler<DrawSketchHandlerPolygon, StateMachines::TwoSeekEnd, 2>
	{
		using SupperClass = DrawSketchDefaultHandler<DrawSketchHandlerPolygon, StateMachines::TwoSeekEnd, 2>;
	public:
		DrawSketchHandlerPolygon(const std::string& name);
		virtual ~DrawSketchHandlerPolygon();
		virtual void onUpdate()override;
		virtual void onKeyPress(const std::string& key)override;
		virtual void updateDataAndDrawToPosition(Base::Vector2d onSketchPos)override;
		bool canGoToNextMode() override;
		void createShape(bool onlyeditoutline) override;
	private:
		unsigned int numberOfCorners;
		double radius;
		Base::Vector2d centerPoint, firstCorner;
	};
}
