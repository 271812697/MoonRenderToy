#pragma once
#include "Interactive/EventWidget.h"
namespace MOON
{
	class SketchPlane: public EventWidget
	{
	public:
		SketchPlane(const std::string& name);
		virtual ~SketchPlane();
		virtual void onUpdate()override;
		virtual void onLeftMousePressed()override;
	private:
		class SketchPlaneInternal;
		SketchPlaneInternal* m_internal = nullptr;
	};
}