#pragma once
#include "Gizmo/GizmoWidget.h"

namespace MOON
{
	class ClipPlane: public GizmoWidget
	{
	public:
		ClipPlane(const std::string& name);
		virtual ~ClipPlane();
		virtual void onUpdate()override;

	private:
		class ClipPlaneInternal;
		ClipPlaneInternal* m_internal = nullptr;
	};
}