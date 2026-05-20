#include "Gizmo/Gizmo.h"
#include "Gizmo/Widgets/DrawSketchHandlerTrimming.h"
#include "renderer/SceneView.h"
#include "Qtimgui/imgui/imgui.h"
#include "core/log.h"


namespace MOON {
	class DrawSketchHandlerTrimming::Internal {
	public:
		Internal(DrawSketchHandlerTrimming* s) :self(s) {
		}
		~Internal() {
		}
	private:
		friend DrawSketchHandlerTrimming;
		DrawSketchHandlerTrimming* self = nullptr;
	};
	DrawSketchHandlerTrimming::DrawSketchHandlerTrimming(const std::string& name) : DrawSketchHandler(name)
	{
		m_internal = new Internal(this);
	}
	DrawSketchHandlerTrimming::~DrawSketchHandlerTrimming()
	{
		delete m_internal;
	}
	void DrawSketchHandlerTrimming::onUpdate()
	{
		DrawSketchHandler::onUpdate();
		
	}
	void DrawSketchHandlerTrimming::onSetActive(bool flag)
	{
		DrawSketchHandler::onSetActive(flag);
		
	}

	void DrawSketchHandlerTrimming::onMouseMove()
	{
		CORE_DEBUG("DrawSketchHandlerTrimming::onMouseMove")
	}

	void DrawSketchHandlerTrimming::onLeftMousePressed()
	{
		CORE_DEBUG("DrawSketchHandlerTrimming::onLeftMousePressed");
	}
}