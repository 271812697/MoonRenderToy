#pragma once
#include "Gizmo/GizmoWidget.h"
namespace MOON
{
	class SketchPlane: public GizmoWidget
	{
	public:
		SketchPlane(const std::string& name);
		virtual ~SketchPlane();
		virtual void onUpdate()override;
		
	private:
		class SketchPlaneInternal;
		SketchPlaneInternal* m_internal = nullptr;
	};
}