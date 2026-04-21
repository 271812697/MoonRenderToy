#include "Gizmo/Widgets/DrawSketchHandlerCircle.h"
#include "Gizmo/Gizmo.h"
#include "renderer/SceneView.h"
#include "Qtimgui/imgui/imgui.h"
#include "Gizmo/Interactive/Event.h"
#include "Gizmo/Interactive/ExecuteCommand.h"
#include "Gizmo/Interactive/WidgetCallbackMapper.h"
#include "Gizmo/Interactive/WidgetEvent.h"
#include "Gizmo/Interactive/WidgetEventTranslator.h"
#include "Gizmo/Interactive/RenderWindowInteractor.h"

namespace MOON {
	class DrawSketchHandlerCircle::DrawSketchHandlerCircleInternal {
	public:
		DrawSketchHandlerCircleInternal(DrawSketchHandlerCircle*s) :self(s){
			pos = { 0,0 };
			radius = 5;
		}
		~DrawSketchHandlerCircleInternal() {
		
		}
	private:
		friend DrawSketchHandlerCircle;
		DrawSketchHandlerCircle* self = nullptr;
		Vec2 pos;
		float radius;
	};
	
	DrawSketchHandlerCircle::DrawSketchHandlerCircle(const std::string& name) :GizmoWidget(name),m_internal(new DrawSketchHandlerCircleInternal(this))
	{
		// Define widget events
		setActive(true);
	}

	DrawSketchHandlerCircle::~DrawSketchHandlerCircle()
	{
		delete m_internal;
	}

	void DrawSketchHandlerCircle::onUpdate()
	{
		renderer->pushSize(3);
		renderer->drawLine2D({100,0},{-100,0});
		renderer->drawLine2D({ 0,100 }, { 0,-100 });
		renderer->drawCircle2D(m_internal->pos,m_internal->radius);
		renderer->popSize();
	}

	void DrawSketchHandlerCircle::onSetActive(bool flag)
	{
	}

	void DrawSketchHandlerCircle::onMouseClicked()
	{
	}

	void DrawSketchHandlerCircle::onMouseMove()
	{
	}

	void DrawSketchHandlerCircle::SetEnabled(int)
	{
	}

}