#pragma once
#include "Interactive/Widgets/DrawSketchDefaultHandler.h"
#include <TopoDS_Wire.hxx>
namespace MOON
{
	namespace ConstructionMethods
	{
		enum class OffsetConstructionMethod
		{
			Arc,
			Intersection,
			End // Must be the last one
		};
	}

	// Sketch offset tool (ported from FreeCAD SketcherGui::DrawSketchHandlerOffset).
	// Works on the currently selected sketch geometry: after activation a single
	// click sets the offset distance/direction and the offset curves are added.
	class DrawSketchHandlerOffset
		: public DrawSketchDefaultHandler<DrawSketchHandlerOffset, StateMachines::OneSeekEnd, 0, ConstructionMethods::OffsetConstructionMethod>
	{
		using SupperClass = DrawSketchDefaultHandler<DrawSketchHandlerOffset, StateMachines::OneSeekEnd, 0, ConstructionMethods::OffsetConstructionMethod>;
	public:
		DrawSketchHandlerOffset(const std::string& name);
		virtual ~DrawSketchHandlerOffset() override;
		virtual void onUpdate() override;
		virtual void onSetActive(bool flag) override;
		virtual void updateDataAndDrawToPosition(Base::Vector2d onSketchPos) override;
		virtual void executeCommands() override;
		virtual void onKeyPress(const std::string& key) override;

	private:
		void generateSourceWires();
		bool findOffsetLength();
		void buildOffsetGeometry();
		void deleteOriginalGeos();

	private:
		std::vector<int> listOfGeoIds;
		std::vector<TopoDS_Wire> sourceWires;
		Base::Vector2d endpoint;
		bool onlySingleLines = true;
		bool deleteOriginal = false;
		bool offsetLengthSet = false;
		double offsetLength = 0.0;
	};
}
