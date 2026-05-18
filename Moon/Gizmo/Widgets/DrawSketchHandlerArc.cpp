#include "Gizmo/Widgets/DrawSketchHandlerArc.h"
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
    static Base::Vector2d getCircleCenter(
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
            //THROWM(Base::ValueError, "Two points are coincident");
        }

        double uv = -(u * v);
        double vw = -(v * w);
        double uw = -(u * w);

        double w0 = (2 * sqrt(abs(uu * ww - uw * uw)) * uw / (uu * ww));
        double w1 = (2 * sqrt(abs(uu * vv - uv * uv)) * uv / (uu * vv));
        double w2 = (2 * sqrt(abs(vv * ww - vw * vw)) * vw / (vv * ww));

        double wx = w0 + w1 + w2;

        if (abs(wx) < Precision::Confusion()) {
            //THROWM(Base::ValueError, "Points are collinear");
        }

        double x = (w0 * p1.x + w1 * p2.x + w2 * p3.x) / wx;
        double y = (w0 * p1.y + w1 * p2.y + w2 * p3.y) / wx;

        return { x, y };
    }
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
	class DrawSketchHandlerArc::DrawSketchHandlerArcInternal {
	public:
        DrawSketchHandlerArcInternal(DrawSketchHandlerArc*s) :
            self(s), 
            radius(0.0)
            , startAngle(0.0)
            , endAngle(0.0)
            , arcAngle(0.0) {
		}
		~DrawSketchHandlerArcInternal() {
		
		}
	private:
		friend DrawSketchHandlerArc;
        DrawSketchHandlerArc* self = nullptr;
        Base::Vector2d centerPoint, firstPoint, secondPoint;
        double radius, startAngle, endAngle, arcAngle;
	};
	
    DrawSketchHandlerArc::DrawSketchHandlerArc(const std::string& name) :DrawSketchDefaultHandler<DrawSketchHandlerArc, StateMachines::ThreeSeekEnd, 3, CircleEllipseConstructionMethod>(name),m_internal(new DrawSketchHandlerArcInternal(this))
	{

	}

    DrawSketchHandlerArc::~DrawSketchHandlerArc()
	{
		delete m_internal;
	}

	void DrawSketchHandlerArc::onUpdate()
	{
        DrawSketchHandler::onUpdate();

        for (int i = 0; i < lines.size();i+=2) {
            renderer->drawLine2D({lines[i].x
                ,lines[i].y}, { lines[i+1].x
                ,lines[i+1].y },static_cast<MOON::Plane2D>(plane));
        }
	}

	void DrawSketchHandlerArc::onSetActive(bool flag)
	{
	}





	void DrawSketchHandlerArc::updateDataAndDrawToPosition(Base::Vector2d onSketchPos)
	{
        switch (state()) {
        case SelectMode::SeekFirst: {
            //toolWidgetManager.drawPositionAtCursor(onSketchPos);

            if (constructionMethod() == ConstructionMethod::Center) {
                m_internal->centerPoint = onSketchPos;
            }
            else {
                m_internal->firstPoint = onSketchPos;
            }

            //seekAndRenderAutoConstraint(sugConstraints[0], onSketchPos, Base::Vector2d(0.f, 0.f));
        } break;
        case SelectMode::SeekSecond: {
            if (constructionMethod() == ConstructionMethod::Center) {
                m_internal->firstPoint = onSketchPos;
                m_internal->startAngle = (m_internal->firstPoint - m_internal->centerPoint).Angle();
            }
            else {
                m_internal->centerPoint = (onSketchPos - m_internal->firstPoint) / 2 + m_internal->firstPoint;
                m_internal->secondPoint = onSketchPos;
            }

            m_internal->radius = (onSketchPos - m_internal->centerPoint).Length();

            CreateAndDrawShapeGeometry();

            if (constructionMethod() == ConstructionMethod::Center) {
                //toolWidgetManager.drawDirectionAtCursor(onSketchPos, centerPoint);
            }
            else {
                //toolWidgetManager.drawPositionAtCursor(onSketchPos);
            }

            //seekAndRenderAutoConstraint(sugConstraints[1], onSketchPos, Base::Vector2d(0.f, 0.f));
        } break;
        case SelectMode::SeekThird: 
        {
            double startAngleBackup = m_internal->startAngle;

            if (constructionMethod() == ConstructionMethod::Center) {
                m_internal->secondPoint = onSketchPos;
                double angle1 = (onSketchPos - m_internal->centerPoint).Angle() - m_internal->startAngle;
                double angle2 = angle1 + (angle1 < 0. ? 2 : -2) * 3.1415926535;
                m_internal->arcAngle = abs(angle1 - m_internal->arcAngle) < abs(angle2 - m_internal->arcAngle) ? angle1 : angle2;

                if (m_internal->arcAngle > 0) {
                    m_internal->endAngle = m_internal->startAngle + m_internal->arcAngle;
                }
                else {
                    m_internal->endAngle = m_internal->startAngle;
                    m_internal->startAngle = m_internal->startAngle + m_internal->arcAngle;
                }
            }
            else {
                if (areCollinear(m_internal->firstPoint, m_internal->secondPoint, onSketchPos)) {
                    // If points are collinear then we can't calculate the center.
                    return;
                }
                m_internal->centerPoint
                    = getCircleCenter(m_internal->firstPoint, m_internal->secondPoint, onSketchPos);
                m_internal->radius = (onSketchPos - m_internal->centerPoint).Length();

                double angle1 = (m_internal->firstPoint - m_internal->centerPoint).Angle();
                double angle2 = (m_internal->secondPoint - m_internal->centerPoint).Angle();
                double angle3 = (onSketchPos - m_internal->centerPoint).Angle();

                // Always build arc counter-clockwise
                // Point 3 is between Point 1 and 2
                if (angle3 > std::min(angle1, angle2) && angle3 < std::max(angle1, angle2)) {
                    //if (angle2 > angle1) {
                    //    arcPos1 = Sketcher::PointPos::start;
                    //    arcPos2 = Sketcher::PointPos::end;
                    //}
                    //else {
                    //    arcPos1 = Sketcher::PointPos::end;
                    //    arcPos2 = Sketcher::PointPos::start;
                    //}
                    m_internal->startAngle = std::min(angle1, angle2);
                    m_internal->endAngle = std::max(angle1, angle2);
                    m_internal->arcAngle = m_internal->endAngle - m_internal->startAngle;
                }
                // Point 3 is not between Point 1 and 2
                else {
                    //if (angle2 > angle1) {
                    //    arcPos1 = Sketcher::PointPos::end;
                    //    arcPos2 = Sketcher::PointPos::start;
                    //}
                    //else {
                    //    arcPos1 = Sketcher::PointPos::start;
                    //    arcPos2 = Sketcher::PointPos::end;
                    //}
                    m_internal->startAngle = std::max(angle1, angle2);
                    m_internal->endAngle = std::min(angle1, angle2);
                    m_internal->arcAngle = 2 * 3.1415926535 - (m_internal->startAngle - m_internal->endAngle);
                }
            }

            CreateAndDrawShapeGeometry();

            if (constructionMethod() == ConstructionMethod::Center) {
                m_internal->startAngle = startAngleBackup;
                //toolWidgetManager.drawDoubleAtCursor(onSketchPos, arcAngle, Base::Unit::Angle);
                //seekAndRenderAutoConstraint(sugConstraints[2], onSketchPos, Base::Vector2d(0.0, 0.0));
            }
            else {
                //toolWidgetManager.drawPositionAtCursor(onSketchPos);
                //seekAndRenderAutoConstraint(
                //    sugConstraints[2],
                //    onSketchPos,
                //    Base::Vector2d(0.f, 0.f),
                //    AutoConstraint::CURVE
                //);
            }

        } break;
        default:
            break;
        }
	}



    bool DrawSketchHandlerArc::canGoToNextMode()
    {
        if (state() == SelectMode::SeekSecond && m_internal->radius < Precision::Confusion()) {
            // Prevent validation of null arc.
            return false;
        }
        if (state() == SelectMode::SeekThird && fabs(m_internal->arcAngle) < Precision::Confusion()) {
            return false;
        }
        return true;
    }

    void DrawSketchHandlerArc::createShape(bool onlyeditoutline)
    {
        ShapeGeometry.clear();

        if (m_internal->radius < Precision::Confusion()) {
            return;
        }

        if (state() == SelectMode::SeekSecond) {
            addCircleToShapeGeometry(toVector3d(m_internal->centerPoint), m_internal->radius, true);
        }
        else {
            if (fabs(m_internal->arcAngle) < Precision::Confusion()) {
                return;
            }

            addArcToShapeGeometry(
                toVector3d(m_internal->centerPoint),
                m_internal->startAngle,
                m_internal->endAngle,
                m_internal->radius,
                true
            );
        }

        if (onlyeditoutline) {
            if (constructionMethod() == ConstructionMethod::Center) {
                if (state() == SelectMode::SeekThird) {
                    const double scale = 0.8;
                    addLineToShapeGeometry(
                        toVector3d(m_internal->centerPoint),
                        Base::Vector3d(
                            m_internal->centerPoint.x + cos(m_internal->startAngle) * scale * m_internal->radius,
                            m_internal->centerPoint.y + sin(m_internal->startAngle) * scale * m_internal->radius,
                            0.
                        ),
                        true
                    );

                    addLineToShapeGeometry(
                        toVector3d(m_internal->centerPoint),
                        Base::Vector3d(
                            m_internal->centerPoint.x + cos(m_internal->endAngle) * scale * m_internal->radius,
                            m_internal->centerPoint.y + sin(m_internal->endAngle) * scale * m_internal->radius,
                            0.
                        ),
                        true
                    );
                }
            }
            else {
                if (state() == SelectMode::SeekSecond) {
                    addLineToShapeGeometry(
                        toVector3d(m_internal->firstPoint),
                        toVector3d(m_internal->secondPoint),
                        true
                    );
                }
                else if (state() == SelectMode::SeekThird) {
                    const double scale = 0.8;
                    addLineToShapeGeometry(
                        toVector3d(m_internal->centerPoint),
                        toVector3d(m_internal->centerPoint)
                        + (toVector3d(m_internal->secondPoint) - toVector3d(m_internal->centerPoint)) * scale,
                        true
                    );

                    addLineToShapeGeometry(
                        toVector3d(m_internal->centerPoint),
                        toVector3d(m_internal->centerPoint)
                        + (toVector3d(m_internal->firstPoint) - toVector3d(m_internal->centerPoint)) * scale,
                        true
                    );
                }
            }
        }
    }

}