#pragma once
#include "Gizmo/GizmoWidget.h"

#include "Geometry.h"
#include "Maths/FVector3.h"
#include "base/Tools2D.h"
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
        void makePlane(int v);
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
        bool drawSketchPos = false;
        Base::Vector2d drawPos;
        Base::Vector2d onSketchPos;
        int plane = 2;//0->X,1->Y,2->Z;
        Maths::FVector3 planeNormal;
    };
}