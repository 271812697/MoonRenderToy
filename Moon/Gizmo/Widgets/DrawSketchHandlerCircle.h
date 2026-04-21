#pragma once
#include "Gizmo/GizmoWidget.h"

namespace MOON
{
 
	class DrawSketchHandlerCircle: public GizmoWidget
	{
	public:
		DrawSketchHandlerCircle(const std::string& name);
		virtual ~DrawSketchHandlerCircle();
		virtual void onUpdate()override;
		virtual void onSetActive(bool flag)override;
		void onMouseClicked();
		void onMouseMove();
		void SetEnabled(int) override;
	private:
		class DrawSketchHandlerCircleInternal;
		DrawSketchHandlerCircleInternal* m_internal = nullptr;
	};
}