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
    static Base::Vector3d toVector3d(const Vec2& vector2d)
    {
        return Base::Vector3d(vector2d.x(), vector2d.y(), 0.);
    }
	class DrawSketchHandlerCircle::DrawSketchHandlerCircleInternal {
	public:
		DrawSketchHandlerCircleInternal(DrawSketchHandlerCircle*s) :self(s){
            centerPoint = { 0,0 };
			radius = 5;
		}
		~DrawSketchHandlerCircleInternal() {
		
		}
	private:
		friend DrawSketchHandlerCircle;
		DrawSketchHandlerCircle* self = nullptr;
		
		float radius;
        Vec2 centerPoint, firstPoint, secondPoint;
       
        bool isDiameter;
	};
	
	DrawSketchHandlerCircle::DrawSketchHandlerCircle(const std::string& name) :DrawSketchDefaultHandler<DrawSketchHandlerCircle, StateMachines::ThreeSeekEnd, 3, CircleEllipseConstructionMethod>(name),m_internal(new DrawSketchHandlerCircleInternal(this))
	{

	}

	DrawSketchHandlerCircle::~DrawSketchHandlerCircle()
	{
		delete m_internal;
	}

	void DrawSketchHandlerCircle::onUpdate()
	{
        DrawSketchHandler::onUpdate();
        if (state() == SelectMode::SeekSecond) {
			drawFloatValue(m_internal->radius);
        }
	}

	void DrawSketchHandlerCircle::onSetActive(bool flag)
	{
	}





	void DrawSketchHandlerCircle::updateDataAndDrawToPosition(Base::Vector2d onSketchPos)
	{
        switch (state()) {
        case SelectMode::SeekFirst: {
            drawPositionAtCursor(onSketchPos);
            if (constructionMethod() == ConstructionMethod::Center) {
                m_internal->centerPoint = { onSketchPos.x,onSketchPos.y };
                //seekAndRenderAutoConstraint(sugConstraints[0], onSketchPos, Base::Vector2d());
            }
            else {
                m_internal->firstPoint = { onSketchPos.x,onSketchPos.y };
                //seekAndRenderAutoConstraint(
                //    sugConstraints[0],
                //    onSketchPos,
                //    Base::Vector2d(),
                //    AutoConstraint::CURVE
                //);
            }
        } break;
        case SelectMode::SeekSecond: {
            //drawPositionAtCursor(onSketchPos);
            clearPositionAtCursor();
            if (constructionMethod() == ConstructionMethod::ThreeRim) {
                m_internal->centerPoint = (Vec2(onSketchPos.x,onSketchPos.y) - m_internal->firstPoint) / 2 + m_internal->firstPoint;
            }
            m_internal->secondPoint = { onSketchPos.x,onSketchPos.y };
            m_internal->radius = (Vec2(onSketchPos.x, onSketchPos.y) - m_internal->centerPoint).norm();
            CreateAndDrawShapeGeometry();
            if (constructionMethod() == ConstructionMethod::Center) {
               // toolWidgetManager.drawDoubleAtCursor(onSketchPos, radius);
            }
            else {
                //toolWidgetManager.drawPositionAtCursor(onSketchPos);
            }

            //seekAndRenderAutoConstraint(
            //    sugConstraints[1],
            //    onSketchPos,
            //    constructionMethod() == ConstructionMethod::Center ? onSketchPos - centerPoint
            //    : Base::Vector2d(),
            //    AutoConstraint::CURVE
            //);
        } break;
        case SelectMode::SeekThird: {
            //try {
            //    if (areCollinear(firstPoint, secondPoint, onSketchPos)) {
            //        // If points are collinear then we can't calculate the center.
            //        return;
            //    }

            //    centerPoint
            //        = Part::Geom2dCircle::getCircleCenter(firstPoint, secondPoint, onSketchPos);

            //    radius = (onSketchPos - centerPoint).Length();

            //    toolWidgetManager.drawPositionAtCursor(onSketchPos);

            //    CreateAndDrawShapeGeometry();

            //    seekAndRenderAutoConstraint(
            //        sugConstraints[2],
            //        onSketchPos,
            //        Base::Vector2d(0.f, 0.f),
            //        AutoConstraint::CURVE
            //    );
            //}
            //catch (Base::ValueError& e) {
            //    e.reportException();
            //}
        } break;
        default:
            break;
        }
	}

    void DrawSketchHandlerCircle::onButtonPressed(Base::Vector2d onSketchPos)
    {
        this->updateDataAndDrawToPosition(onSketchPos);
        if (canGoToNextMode()) {
            if (state() == SelectMode::SeekSecond
                && constructionMethod() == ConstructionMethod::Center) {
                setState(SelectMode::End);
            }
            else
            {
                moveToNextMode();
            }
        }
    }

    bool DrawSketchHandlerCircle::canGoToNextMode()
    {
        if (state() == SelectMode::SeekSecond && m_internal->radius < Precision::Confusion()) {
            // Prevent validation of null circle.
            return false;
        }
        return true;
    }

    void DrawSketchHandlerCircle::createShape(bool onlyeditoutline)
    {
        ShapeGeometry.clear();
        if (m_internal->radius < Precision::Confusion()) {
            return;
        }
        addCircleToShapeGeometry(toVector3d(m_internal->centerPoint), m_internal->radius,true);
    }

}