#include "Gizmo/Widgets/PadTaskWidget.h"
#include "Gizmo/Gizmo.h"
#include "Gizmo/MathUtil/MathUtil.h"

namespace MOON {
	PadTaskWidget::PadTaskWidget(const std::string& name):GizmoWidget(name)
	{
	}
	PadTaskWidget::~PadTaskWidget()
	{
	}
	void PadTaskWidget::onUpdate()
	{
		renderer->drawPoint({ 0,0,0 }, 50);
	}
}