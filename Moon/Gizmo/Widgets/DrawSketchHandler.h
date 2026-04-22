#pragma once
#include "Gizmo/GizmoWidget.h"

#include "Geometry.h"
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
       
    protected:
        void drawEdit(const std::vector<Base::Vector2d>& EditCurve) ;
        void drawEdit(const std::list<std::vector<Base::Vector2d>>& list) ;
        void drawEdit(const std::vector<Part::Geometry*>& geometries);
    protected:
		std::vector<Base::Vector2d> lines;
		std::vector<Base::Vector2d> points;


    };
}