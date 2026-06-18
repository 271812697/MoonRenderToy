#include "Interactive/Widgets/DrawSketchHandlerCircle.h"
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
    static bool areCollinear(
        const Base::Vector2d& p1,
        const Base::Vector2d& p2,
        const Base::Vector2d& p3
    )
    {
        Base::Vector2d u = p2 - p1;
        Base::Vector2d v = p3 - p2;
        Base::Vector2d w = p1 - p3;

        double uu = u * u;
        double vv = v * v;
        double ww = w * w;

        double eps2 = Precision::SquareConfusion();
        if (uu < eps2 || vv < eps2 || ww < eps2) {
            return true;
        }

        double uv = -(u * v);
        double vw = -(v * w);
        double uw = -(u * w);

        double w0 = (2 * sqrt(abs(uu * ww - uw * uw)) * uw / (uu * ww));
        double w1 = (2 * sqrt(abs(uu * vv - uv * uv)) * uv / (uu * vv));
        double w2 = (2 * sqrt(abs(vv * ww - vw * vw)) * vw / (vv * ww));

        double wx = w0 + w1 + w2;

        if (abs(wx) < Precision::Confusion()) {
            return true;
        }

        return false;
    }

    static Base::Vector3d toVector3d(const Base::Vector2d& vector2d)
    {
        return Base::Vector3d(vector2d.x, vector2d.y, 0.);
    }
	class DrawSketchHandlerCircle::DrawSketchHandlerCircleInternal {
	public:
		DrawSketchHandlerCircleInternal(DrawSketchHandlerCircle*s) :self(s){
            centerPoint = Base::Vector2d{ 0.0,0.0 };
			radius = 5;
		}
		~DrawSketchHandlerCircleInternal() {
		
		}
	private:
		friend DrawSketchHandlerCircle;
		DrawSketchHandlerCircle* self = nullptr;
		
		float radius;
        Base::Vector2d centerPoint, firstPoint, secondPoint;
       
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
                m_internal->centerPoint = (onSketchPos- m_internal->firstPoint) / 2 + m_internal->firstPoint;
            }
            m_internal->secondPoint = { onSketchPos.x,onSketchPos.y };
            m_internal->radius = (onSketchPos- m_internal->centerPoint).Length();
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
            try {
                if (areCollinear(m_internal->firstPoint, m_internal->secondPoint, onSketchPos)) {
                    // If points are collinear then we can't calculate the center.
                    return;
                }

              
                 auto p=    Part::Geom2dCircle::getCircleCenter( m_internal->firstPoint,  m_internal->secondPoint, onSketchPos);
				 m_internal->centerPoint = { p.x, p.y };
                 m_internal->radius = (onSketchPos -m_internal->centerPoint).Length();

                drawPositionAtCursor(onSketchPos);

                CreateAndDrawShapeGeometry();

                //seekAndRenderAutoConstraint(
                //    sugConstraints[2],
                //    onSketchPos,
                //    Base::Vector2d(0.f, 0.f),
                //    AutoConstraint::CURVE
                //);
            }
            catch (Base::ValueError& e) {
                e.reportException();
            }
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