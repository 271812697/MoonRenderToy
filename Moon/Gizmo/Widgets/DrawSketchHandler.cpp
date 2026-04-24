#pragma once
#include "DrawSketchHandler.h"
namespace MOON
{
    static std::vector<Base::Vector2d> toVector2D(const Part::Geometry* geometry)
    {
        int curvedEdgeCountSegments = 50;
        std::vector<Base::Vector2d> vector2d;
        auto emplaceasvector2d = [&vector2d](const Base::Vector3d& point) {
            vector2d.emplace_back(point.x, point.y);
            };
        auto isperiodicconic = geometry->is<Part::GeomCircle>() || geometry->is<Part::GeomEllipse>();
        auto isbounded = geometry->isDerivedFrom<Part::GeomBoundedCurve>();

        if (geometry->is<Part::GeomLineSegment>()) 
        {  // add a line
            auto geo = static_cast<const Part::GeomLineSegment*>(geometry);
            emplaceasvector2d(geo->getStartPoint());
            emplaceasvector2d(geo->getEndPoint());
        }
        else if (isperiodicconic || isbounded) 
        {
            auto geo = static_cast<const Part::GeomConic*>(geometry);
            double segment = (geo->getLastParameter() - geo->getFirstParameter())
                / curvedEdgeCountSegments;
            for (int i = 0; i < curvedEdgeCountSegments; i++) {
                emplaceasvector2d(geo->value(geo->getFirstParameter() + i * segment));
            }
            // either close the curve for untrimmed conic or set the last point for bounded curves
            emplaceasvector2d(isperiodicconic ? geo->value(0) : geo->value(geo->getLastParameter()));
        }
        return vector2d;
    }
    void DrawSketchHandler::quit()
    {
    }
    void DrawSketchHandler::clearEdit()
    {
        lines.clear();
    }
    void DrawSketchHandler::drawEdit(const std::vector<Base::Vector2d>& EditCurve)
    {
        for (int i = 0; i < EditCurve.size() - 1; i++) {
            lines.push_back(EditCurve[i]);
            lines.push_back(EditCurve[i + 1]);
        }
    }
    void DrawSketchHandler::drawEdit(const std::list<std::vector<Base::Vector2d>>& list) 
    {
        for (auto it = list.begin(); it != list.end(); it++) {
            auto& segment = *it;
            for (int i = 0; i < segment.size() - 1;i++) {
                lines.push_back(segment[i]);
                lines.push_back(segment[i+1]);
            } 
        }
    }

    void DrawSketchHandler::drawEdit(const std::vector<Part::Geometry*>& geometries)  {
        std::list<std::vector<Base::Vector2d>> list;
        for (const auto& geo : geometries) {
            list.push_back(toVector2D(geo));
        }
        drawEdit(list);
    }
}