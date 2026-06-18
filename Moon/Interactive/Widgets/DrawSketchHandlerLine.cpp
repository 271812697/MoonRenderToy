#include "Interactive/Widgets/DrawSketchHandlerLine.h"
#include "Interactive/Im3DRenderer.h"
#include "renderer/SceneView.h"
#include "Qtimgui/imgui/imgui.h"
#include "Interactive/Interactive/Event.h"
#include "Interactive/Interactive/ExecuteCommand.h"
#include "Interactive/Interactive/WidgetCallbackMapper.h"
#include "Interactive/Interactive/WidgetEvent.h"
#include "Interactive/Interactive/WidgetEventTranslator.h"
#include "Interactive/Interactive/RenderWindowInteractor.h"
#include "Geometry2d.h"

namespace MOON {
	class DrawSketchHandlerLine::Internal {
	public:
		Internal(DrawSketchHandlerLine*s) :self(s){
            editPoint = Base::Vector2d{ 0.0,0.0 };
		}
		~Internal() {
		
		}
	private:
		friend DrawSketchHandlerLine;
		DrawSketchHandlerLine* self = nullptr;
        Base::Vector2d editPoint;
	};
	
	DrawSketchHandlerLine::DrawSketchHandlerLine(const std::string& name,LineConstructionMethod constrMethod) :SupperClass(name, constrMethod),m_internal(new Internal(this)),  length(0.0)
		, lengthSign(0)
		, widthSign(0)
		, capturedDirection(0.0, 0.0)
	{

	}

	DrawSketchHandlerLine::~DrawSketchHandlerLine()
	{
		delete m_internal;
	}

	void DrawSketchHandlerLine::onUpdate()
	{
        DrawSketchHandler::onUpdate();

		renderer->drawPoint(plane.valueEigen(m_internal->editPoint), 12);
	}

	void DrawSketchHandlerLine::updateDataAndDrawToPosition(Base::Vector2d onSketchPos)
	{
		switch (state()) {
		case SelectMode::SeekFirst: {
			drawPositionAtCursor(onSketchPos);

			startPoint = onSketchPos;

			//seekAndRenderAutoConstraint(sugConstraints[0], onSketchPos, Base::Vector2d(0.f, 0.f));
		} break;
		case SelectMode::SeekSecond: {
			//drawDirectionAtCursor(onSketchPos, startPoint);

			endPoint = onSketchPos;

			try {
				CreateAndDrawShapeGeometry();
			}
			catch (const Base::ValueError&) {
			}  // equal points while hovering raise an objection that can be safely ignored

			//seekAndRenderAutoConstraint(sugConstraints[1], onSketchPos, onSketchPos - startPoint);
		} break;
		default:
			break;
		}
	}


    void DrawSketchHandlerLine::createShape(bool onlyeditoutline)
    {
		Q_UNUSED(onlyeditoutline);
		ShapeGeometry.clear();

		Base::Vector2d vecL = endPoint - startPoint;
		length = vecL.Length();
		if (length > Precision::Confusion()) {

			addLineToShapeGeometry(Base::Vector3d(startPoint.x,startPoint.y,0), Base::Vector3d(endPoint.x,endPoint.y,0), false);
		}
    }

	void DrawSketchHandlerLine::onReset()
	{
		lengthSign = 0;
		widthSign = 0;
		capturedDirection = Base::Vector2d(0.0, 0.0);
	}

}