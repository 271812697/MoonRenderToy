#pragma once
#include "Gizmo/GizmoWidget.h"
#include "Gizmo/Widgets/DrawSketchDefaultHandler.h"
namespace MOON
{
	enum class BSplineConstructionMethod
	{
		ControlPoints,
		Knots,
		End  // Must be the last one
	};

	class DrawSketchHandlerBSpline : public DrawSketchDefaultHandler<DrawSketchHandlerBSpline, StateMachines::ThreeSeekEnd,2, BSplineConstructionMethod>
	{
	public:
		DrawSketchHandlerBSpline(const std::string& name,BSplineConstructionMethod constrMethod = BSplineConstructionMethod::ControlPoints,
			bool periodic = false);
		virtual ~DrawSketchHandlerBSpline();
		virtual void onUpdate()override;
		virtual void onSetActive(bool flag)override;
		void SetEnabled(int) override;
		virtual void updateDataAndDrawToPosition(Base::Vector2d onSketchPos)override;
		//virtual void onButtonPressed(Base::Vector2d onSketchPos) override;
		bool canGoToNextMode() override;
		void quit() override;
		void createShape(bool onlyeditoutline) override;
		bool addPos();
		void addToVectors();
		bool addGeometry(Base::Vector2d pos, int geoId, bool firstPoint);
		Base::Vector2d getLastPoint();
	private:
		size_t SplineDegree;
		bool periodic;
		Base::Vector2d prevCursorPosition;
		std::vector<Base::Vector2d> points;
		std::vector<int> multiplicities;
		std::vector<int> geoIds;
		std::vector<bool> isBetweenC0Points;
		std::vector<double> distances;
		bool resetSeekSecond;
		//class DrawSketchHandlerBSplineInternal;
		//DrawSketchHandlerBSplineInternal* m_internal = nullptr;
	};
}