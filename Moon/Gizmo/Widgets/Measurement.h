#pragma once
#include "Gizmo/GizmoWidget.h"

namespace MOON
{
	class Measurement: public GizmoWidget
	{
	public:
		Measurement(const std::string& name);
		virtual ~Measurement();
		virtual void onUpdate()override;
		virtual void onSetActive(bool flag)override;
		void onMouseClicked();
		void onMouseMove();
		void SetEnabled(int) override;
		static void MousePressed(AbstractWidget*);
		static void mouseMove(AbstractWidget*);
	};
}