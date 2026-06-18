#pragma once
#include "Interactive/EventWidget.h"
#include "Interactive/Widgets/DrawSketchDefaultHandler.h"
namespace MOON
{
	class DrawSketchHandlerPoint: public DrawSketchDefaultHandler<DrawSketchHandlerPoint, StateMachines::OneSeekEnd,1>
	{
		using SupperClass = DrawSketchDefaultHandler<DrawSketchHandlerPoint, StateMachines::OneSeekEnd, 1>;
	public:
		DrawSketchHandlerPoint(const std::string& name);
		virtual ~DrawSketchHandlerPoint();
		virtual void onUpdate()override;
		virtual void updateDataAndDrawToPosition(Base::Vector2d onSketchPos)override;
		void createShape(bool onlyeditoutline) override;
	private:
		class Internal;
		Internal* m_internal = nullptr;
	};
}