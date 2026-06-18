#pragma once
#include "Interactive/EventWidget.h"
#include "Maths/FVector4.h"

namespace MOON
{
	class ClipPlane: public EventWidget
	{
	public:
		ClipPlane(const std::string& name);
		virtual ~ClipPlane();
		virtual void onUpdate()override;
		Maths::FVector4 getClipPlane();
		virtual void onLeftMousePressed()override;
		virtual void onLeftMouseReleased()override;
		virtual void onMouseMove()override;
	private:
		enum IntersectiveState
		{
			Stop,
			Hot,
			AxisT
		};
		enum PickMeshId
		{
			XAxis,
			YAxis,
			ZAxis,
			None
		};
		IntersectiveState mState;
		PickMeshId mPickMesh;
		class ClipPlaneInternal;
		ClipPlaneInternal* m_internal = nullptr;
	};
}