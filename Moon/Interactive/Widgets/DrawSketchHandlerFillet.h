#pragma once
#include "Interactive/EventWidget.h"
#include "Interactive/Widgets/DrawSketchDefaultHandler.h"
namespace MOON
{
	enum class FilletConstructionMethod
	{
		Fillet,
		Chamfer,
		End  // Must be the last one
	};

	class DrawSketchHandlerFillet : public DrawSketchDefaultHandler<DrawSketchHandlerFillet, StateMachines::TwoSeekEnd,0, FilletConstructionMethod>
	{
		using SupperClass = DrawSketchDefaultHandler<DrawSketchHandlerFillet, StateMachines::TwoSeekEnd, 0, FilletConstructionMethod>;
	public:
		DrawSketchHandlerFillet(const std::string& name,ConstructionMethod constrMethod = ConstructionMethod::Fillet);
		virtual ~DrawSketchHandlerFillet();
		virtual void onUpdate()override;
		virtual void updateDataAndDrawToPosition(Base::Vector2d onSketchPos)override;
		bool canGoToNextMode() override;
		void executeCommands() override;
		void onButtonPressed(Base::Vector2d onSketchPos) override;
	private:
		bool preserveCorner;
		int geoId1, geoId2;
		Base::Vector2d firstPos, secondPos;
	};
}