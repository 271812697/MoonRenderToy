#pragma once
#include "Interactive/EventWidget.h"
#include "Sketcher/SketchePlane2D.h"
namespace MOON
{
	enum SketchPlaneEvent
	{
		SelectPlane=1001,
		NO_Plane
	};
	class SketchPlane: public EventWidget
	{
	public:
		enum Plane
		{
			XY_Plane,
			YZ_Plane,
			XZ_Plane,
			NO_Plane
		};
		SketchPlane(const std::string& name);
		virtual ~SketchPlane();
		virtual void onUpdate()override;
		virtual void onLeftMousePressed()override;
		virtual void onMouseMove()override;
		SketcherPlane2D getSelectPlane();
	private:
		class SketchPlaneInternal;
		SketchPlaneInternal* m_internal = nullptr;
	};
}