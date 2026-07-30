#pragma once
#include "Interactive/Widgets/DrawSketchHandler.h"
#include "Sketcher/SketcherObj.h"
namespace MOON
{
	class DrawSketchHandlerLineSet : public DrawSketchHandler
	{
	public:
		DrawSketchHandlerLineSet(const std::string& name);
		virtual ~DrawSketchHandlerLineSet()override;
		virtual void onUpdate()override;
		virtual void onLeftMousePressed()override;
		virtual void onLeftMouseReleased()override;
		virtual void onRightMousePressed()override;
		virtual void onRightMouseReleased()override;
		virtual void onMouseMove()override;
		virtual void onKeyPress(const std::string& key)override;
		virtual void onKeyRelease(const std::string& key)override;
        void onButtonPressed(Base::Vector2d pos);
        void onButtonReleased(Base::Vector2d pos);
        void mouseMove(Base::Vector2d pos);
        virtual void quit()override;
        void updateTransitionData(int GeoId, SketcherObj::PointPos PosId);
        /// mode table
        enum SELECT_MODE
        {
            STATUS_SEEK_First,
            STATUS_SEEK_Second,
            STATUS_Do,
            STATUS_Close
        };

        enum SEGMENT_MODE
        {
            SEGMENT_MODE_Arc,
            SEGMENT_MODE_Line
        };

        enum TRANSITION_MODE
        {
            TRANSITION_MODE_Free,
            TRANSITION_MODE_Tangent,
            TRANSITION_MODE_Perpendicular_L,
            TRANSITION_MODE_Perpendicular_R
        };

        enum SNAP_MODE
        {
            SNAP_MODE_Free,
            SNAP_MODE_45Degree
        };
    protected:
        SELECT_MODE Mode;
        SEGMENT_MODE SegmentMode;
        TRANSITION_MODE TransitionMode;
        SNAP_MODE SnapMode;
        bool suppressTransition;

        std::vector<Base::Vector2d> EditCurve;
        int firstCurve;
        int previousCurve;
        SketcherObj::PointPos firstPosId;
        SketcherObj::PointPos previousPosId;
        // the latter stores those constraints that a first point would have been given in absence of
        // the transition mechanism
       // std::vector<AutoConstraint> sugConstr1, sugConstr2, virtualsugConstr1;

        Base::Vector2d CenterPoint;
        Base::Vector3d dirVec;
        double startAngle, endAngle, arcRadius;

        bool firstsegment;
        bool ctrlDown = false;
	};
}