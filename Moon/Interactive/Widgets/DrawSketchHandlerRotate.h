#pragma once
#include "Interactive/EventWidget.h"
#include "Interactive/Widgets/DrawSketchDefaultHandler.h"
namespace MOON
{

	class DrawSketchHandlerRotate: public DrawSketchDefaultHandler<DrawSketchHandlerRotate, StateMachines::ThreeSeekEnd,0>
	{
		using SupperClass = DrawSketchDefaultHandler<DrawSketchHandlerRotate, StateMachines::ThreeSeekEnd, 0>;
	public:
		DrawSketchHandlerRotate(const std::string& name);
		virtual ~DrawSketchHandlerRotate();
		virtual void onUpdate()override;
		virtual void updateDataAndDrawToPosition(Base::Vector2d onSketchPos)override;
		void createShape(bool onlyeditoutline) override;
		void deleteOriginalGeos();
		bool canGoToNextMode() override;
		void executeCommands() override;
		virtual void onKeyPress(const std::string& key)override;
		//void onReset() override;
	private:
		std::vector<int> listOfGeoIds;
		Base::Vector2d centerPoint, startPoint, endPoint;

		bool deleteOriginal, cloneConstraints;
		double length, startAngle, endAngle, totalAngle, individualAngle;
		int numberOfCopies;
	};
}