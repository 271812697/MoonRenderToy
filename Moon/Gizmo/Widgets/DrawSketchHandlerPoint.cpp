#include "Gizmo/Widgets/DrawSketchHandlerPoint.h"
#include "Gizmo/Gizmo.h"
#include "renderer/SceneView.h"
#include "Qtimgui/imgui/imgui.h"
#include "Gizmo/Interactive/Event.h"
#include "Gizmo/Interactive/ExecuteCommand.h"
#include "Gizmo/Interactive/WidgetCallbackMapper.h"
#include "Gizmo/Interactive/WidgetEvent.h"
#include "Gizmo/Interactive/WidgetEventTranslator.h"
#include "Gizmo/Interactive/RenderWindowInteractor.h"
#include "Geometry2d.h"

namespace MOON {
	class DrawSketchHandlerPoint::Internal {
	public:
		Internal(DrawSketchHandlerPoint*s) :self(s){
            editPoint = Base::Vector2d{ 0.0,0.0 };
		}
		~Internal() {
		
		}
	private:
		friend DrawSketchHandlerPoint;
		DrawSketchHandlerPoint* self = nullptr;
        Base::Vector2d editPoint;
	};
	
	DrawSketchHandlerPoint::DrawSketchHandlerPoint(const std::string& name) :SupperClass(name),m_internal(new Internal(this))
	{

	}

	DrawSketchHandlerPoint::~DrawSketchHandlerPoint()
	{
		delete m_internal;
	}

	void DrawSketchHandlerPoint::onUpdate()
	{
        DrawSketchHandler::onUpdate();
		renderer->drawPoint2D(Eigen::Vector2f(m_internal->editPoint.x, m_internal->editPoint.y),12,gizmoPlane);
	}

	void DrawSketchHandlerPoint::updateDataAndDrawToPosition(Base::Vector2d onSketchPos)
	{
		switch (state()) {
		case SelectMode::SeekFirst: {
			m_internal->editPoint = onSketchPos;
			drawPositionAtCursor(onSketchPos);
			CreateAndDrawShapeGeometry();
		} break;
		default:
			break;
		}
	}


    void DrawSketchHandlerPoint::createShape(bool onlyeditoutline)
    {
        ShapeGeometry.clear();
		addPointToShapeGeometry(Base::Vector3d(m_internal->editPoint.x, m_internal->editPoint.y,0.0),false);
    }

}