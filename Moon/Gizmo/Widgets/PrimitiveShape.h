#pragma once
#include "Gizmo/GizmoWidget.h"
#include <Eigen/Core>

namespace MOON
{
	class PrimitiveShape : public GizmoWidget
	{
	public:
		PrimitiveShape(const std::string& name);
		virtual ~PrimitiveShape()override;
		virtual void createTopoShape();
		
		virtual void onKeyPress(const std::string& key)override;
	};
}