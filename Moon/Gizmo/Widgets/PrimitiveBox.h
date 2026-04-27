#pragma once
#include "Gizmo/GizmoWidget.h"
#include <Eigen/Core>

namespace MOON
{
	class PrimitiveBox : public GizmoWidget
	{
	public:
		PrimitiveBox(const std::string& name);
		virtual ~PrimitiveBox()override;
		virtual void onUpdate()override;
		virtual void onSetActive(bool flag)override;
		void onMouseClicked();
		void onMouseMove();
		void onRightButtonRelease();
		void SetEnabled(int) override;
		static void MousePressed(AbstractWidget*);
		static void RightMouseReleased(AbstractWidget*);
		static void MouseMove(AbstractWidget*);
	private:
		Eigen::Vector3f translation;
		Eigen::Vector3f scale;
		Eigen::Matrix3f rot;
	};
}