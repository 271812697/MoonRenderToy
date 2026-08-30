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
		virtual void onSetActive(bool flag)override;
		Maths::FVector4 getClipPlane();
		virtual void onLeftMousePressed()override;
		virtual void onLeftMouseReleased()override;
		virtual void onMouseMove()override;	
		enum PickMeshId
		{
			XAxis,
			YAxis,
			ZAxis,
			XNormalPlane,
			YNormalPlane,
			ZNormalPlane,
			XNormalRotate,
			YNormalRotate,
			ZNormalRotate,
			None
		};	
		enum IntersectiveState
		{
			Stop,
			Hot,
			AxisT,
			PlaneT,
			AxisR
		};
	private:
		void updateSection();
	private:
		IntersectiveState mState;
		int mPickMesh=None;
		class ClipPlaneInternal;
		ClipPlaneInternal* m_internal = nullptr;
	};
}