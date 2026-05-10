#pragma once
#include "Gizmo/GizmoWidget.h"
#include <Eigen/Core>

namespace MOON
{
	class PrimitiveShape : public GizmoWidget
	{
	public:
		PrimitiveShape(const std::string& name);
		virtual ~PrimitiveShape()override;
		
		virtual void createTopoShape();
		void SetEnabled(int) override;
	protected:
		
		CallbackCommand* KeyEventCallbackCommand;
		static void ProcessKeyEvents(GizmoObject*, unsigned long, void*, void*);
	};
}