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
        //double radius;
        bool isDiameter;
	};
	
	DrawSketchHandlerCircle::DrawSketchHandlerCircle(const std::string& name) :DrawSketchDefaultHandler<DrawSketchHandlerCircle, StateMachines::ThreeSeekEnd, 3, CircleEllipseConstructionMethod>(name),m_internal(new DrawSketchHandlerCircleInternal(this))
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
		renderer->drawCircle2D(m_internal->centerPoint,m_internal->radius);
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

	void DrawSketchHandlerCircle::updateDataAndDrawToPosition(Vec2 onSketchPos)
	{
        switch (state()) {
        case SelectMode::SeekFirst: {
            //toolWidgetManager.drawPositionAtCursor(onSketchPos);
            if (constructionMethod() == ConstructionMethod::Center) {
                m_internal->centerPoint = onSketchPos;

                //seekAndRenderAutoConstraint(sugConstraints[0], onSketchPos, Base::Vector2d());
            }
            else {
                m_internal->firstPoint = onSketchPos;

                //seekAndRenderAutoConstraint(
                //    sugConstraints[0],
                //    onSketchPos,
                //    Base::Vector2d(),
                //    AutoConstraint::CURVE
                //);
            }
        } break;
        case SelectMode::SeekSecond: {
            if (constructionMethod() == ConstructionMethod::ThreeRim) {
                m_internal->centerPoint = (onSketchPos - m_internal->firstPoint) / 2 + m_internal->firstPoint;
            }
            m_internal->secondPoint = onSketchPos;

            m_internal->radius = (onSketchPos - m_internal->centerPoint).norm();

            //CreateAndDrawShapeGeometry();

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

    void DrawSketchHandlerCircle::onButtonPressed(Vec2 onSketchPos)
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
        if (state() == SelectMode::SeekSecond && m_internal->radius < 1e-7) {
            // Prevent validation of null circle.
            return false;
        }

        return true;
        
    }

}