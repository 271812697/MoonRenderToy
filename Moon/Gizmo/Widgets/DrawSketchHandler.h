#pragma once
#include "Gizmo/GizmoWidget.h"
#include "Geometry.h"
#include "Maths/FVector3.h"
#include "Sketcher/SketchePlane2D.h"
#include <type_traits>
#include <optional>
namespace MOON
{
  
    
    class DrawSketchHandler : public GizmoWidget {
    public:
		DrawSketchHandler(const std::string& name)
			: GizmoWidget(name)
		{
		}
        virtual void quit();
        void clearEdit() ;
        void makePlane(const SketcherPlane2D& v);
        virtual void onUpdate()override;
        virtual void onMouseMove()override;
    protected:
        void drawEdit(const std::vector<Base::Vector2d>& EditCurve) ;
        void drawEdit(const std::list<std::vector<Base::Vector2d>>& list) ;
        void drawEdit(const std::vector<Part::Geometry*>& geometries);
		void drawPositionAtCursor(Base::Vector2d pos);
		void clearPositionAtCursor();
        void drawFloatValue(float value);
        int getPreselectCurve() const;
		Maths::FVector3 getWorldPosFromSketchPos(Base::Vector2d sketchPos);
        
    protected:
		std::vector<Base::Vector2d> lines;
		std::vector<Base::Vector2d> points;
		bool isSnapedSketchPos = false;
        bool drawSketchPos = false;
        Base::Vector2d drawPos;
        Base::Vector2d onSketchPos;
        SketcherPlane2D plane ;//0->X,1->Y,2->Z;
    };
}