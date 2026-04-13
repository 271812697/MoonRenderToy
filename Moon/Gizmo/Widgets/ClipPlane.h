#pragma once
#include "Gizmo/GizmoWidget.h"
#include "Maths/FVector4.h"

namespace MOON
{
	class ClipPlane: public GizmoWidget
	{
	public:
		ClipPlane(const std::string& name);
		virtual ~ClipPlane();
		virtual void onUpdate()override;
		Maths::FVector4 getClipPlane();

	private:
		class ClipPlaneInternal;
		ClipPlaneInternal* m_internal = nullptr;
	};
}