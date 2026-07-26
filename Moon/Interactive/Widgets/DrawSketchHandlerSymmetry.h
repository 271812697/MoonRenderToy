#pragma once
#include "Interactive/EventWidget.h"
#include "Interactive/Widgets/DrawSketchDefaultHandler.h"
namespace MOON
{
	class DrawSketchHandlerSymmetry : public DrawSketchDefaultHandler<DrawSketchHandlerSymmetry, StateMachines::OneSeekEnd,0>
	{
		using SupperClass = DrawSketchDefaultHandler<DrawSketchHandlerSymmetry, StateMachines::OneSeekEnd, 0>;
	public:
		DrawSketchHandlerSymmetry(const std::string& name);
		virtual ~DrawSketchHandlerSymmetry();
		virtual void onUpdate()override;
		virtual void updateDataAndDrawToPosition(Base::Vector2d onSketchPos)override;
		void createShape(bool onlyeditoutline) override;
		void deleteOriginalGeos();
		bool canGoToNextMode() override;
		void executeCommands() override;
	private:
		std::vector<int> listOfGeoIds;
		int refGeoId;
		bool deleteOriginal, createSymConstraints;
	};
}