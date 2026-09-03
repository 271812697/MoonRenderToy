#pragma once
#include "Interactive/EventWidget.h"
#include "Interactive/Widgets/DrawSketchDefaultHandler.h"
namespace MOON
{
	enum class RectangleConstructionMethod
	{
		Diagonal,
		CenterAndCorner,
		ThreePoints,
		CenterAnd3Points,
		End  // Must be the last one
	};

	class DrawSketchHandlerRectangle : public DrawSketchDefaultHandler<DrawSketchHandlerRectangle, StateMachines::FiveSeekEnd,3, RectangleConstructionMethod>
	{
		using SupperClass = DrawSketchDefaultHandler<DrawSketchHandlerRectangle, StateMachines::FiveSeekEnd, 3, RectangleConstructionMethod>;
	public:
		DrawSketchHandlerRectangle(const std::string& name);
		virtual ~DrawSketchHandlerRectangle();
		virtual void onUpdate()override;
		virtual void onSetActive(bool flag)override;
		virtual void updateDataAndDrawToPosition(Base::Vector2d onSketchPos)override;
		virtual void onButtonPressed(Base::Vector2d onSketchPos) override;
		bool canGoToNextMode() override;
		void createShape(bool onlyeditoutline) override;
		void executeCommands() override;
		void onReset() override;
	private:
		Base::Vector2d center, corner1, corner2, corner3, corner4, frameCorner1, frameCorner2,
			frameCorner3, frameCorner4, corner2Initial;
		Base::Vector3d center1, center2, center3, center4;
		bool roundCorners = true, makeFrame = false, cornersReversed = false;
		double radius = 0., length = 0., width = 0., thickness = 0., radiusFrame = 0., angle = 0.,
			angle123 = 0., angle412 = 0.;
		int firstCurve = -1, constructionPointOneId = -1, constructionPointTwoId = -1,
			constructionPointThreeId = -1, centerPointId = -1, side = 0;

		// Sign tracking for OVP lock fix (issue #23459)
		// These store the direction sign when OVP is first set to prevent sign flipping
		int lengthSign, widthSign;
		class DrawSketchHandlerRectangleInternal;
		DrawSketchHandlerRectangleInternal* m_internal = nullptr;
		int getPointSideOfVector(
			Base::Vector2d pointToCheck,
			Base::Vector2d separatingVector,
			Base::Vector2d pointOnVector
		);
		void calculateRadius(Base::Vector2d onSketchPos);
		void calculateThickness(Base::Vector2d onSketchPos);
		void createFirstRectangleGeometries(Base::Vector2d vecL, Base::Vector2d vecW, double L1, double L2);
		void createFirstRectangleLines(Base::Vector2d vecL, Base::Vector2d vecW, double L1, double L2);
		void createFirstRectangleFillets(Base::Vector2d vecL, Base::Vector2d vecW, double L1, double L2);
		void createSecondRectangleGeometries(Base::Vector2d vecL, Base::Vector2d vecW, double L1, double L2);
		void finishOblongCreation(bool thicknessNotZero, bool negThicknessEqualRadius);
		void finishOblongFrameCase2();
		void finishOblongThreePoints(bool thicknessNotZero, bool negThicknessEqualRadius);
		void finishOblongCenterAnd3Points(bool thicknessNotZero, bool negThicknessEqualRadius);
		void finishOblongCenterAndCorner(bool thicknessNotZero, bool negThicknessEqualRadius);
		void finishOblongDiagonal(bool thicknessNotZero, bool negThicknessEqualRadius);
		void finishRectangleCreation(bool thicknessNotZero);
		void finishRectangleFrameCreation();
		void addRectangleFrameConstructionLines();
		void finishCenteredRectangleCreation(bool thicknessNotZero);
		void addRectangleAutoConstraints();
		void addRoundedRectangleAutoConstraints();
		void addCornerCoincidences(int geoId);
		void addAlignmentConstraints();
		void addTangentCoincidences(int geoId);
		void addArcEqualities();
	};
}
