#pragma once
#include "Gizmo/GizmoWidget.h"

namespace MOON
{
	class PadTaskWidget: public GizmoWidget
	{
	public:
		PadTaskWidget(const std::string& name);
		virtual ~PadTaskWidget();
		virtual void onUpdate()override;

	private:
		class Internal;
		Internal* m_internal = nullptr;
	};
}