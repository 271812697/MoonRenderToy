#include "Sketcher/SketcheTool2D.h"
#include "Geometry.h"
namespace MOON {
	std::vector<Base::Vector2d> CurveConvert::toVector2D(const Part::Geometry* geometry, int curvedEdgeCountSegments) {
	
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
    void CurveConvert::toVector2D(const Part::Geometry* geometry, int curvedEdgeCountSegments, std::vector<Base::Vector3d>& vector2d, std::vector<double>& params)
    {
        vector2d.clear();
        params.clear();
        auto emplaceasvector2d = [&vector2d,&params](const Base::Vector3d& point,const double& u) {
            vector2d.emplace_back(point);
			params.push_back(u);
			
            };
        auto isperiodicconic = geometry->is<Part::GeomCircle>() || geometry->is<Part::GeomEllipse>();
        auto isbounded = geometry->isDerivedFrom<Part::GeomBoundedCurve>();
        if (geometry->is<Part::GeomLineSegment>())
        {  // add a line
            auto geo = static_cast<const Part::GeomLineSegment*>(geometry);
            emplaceasvector2d(geo->getStartPoint(),geo->getFirstParameter());
            emplaceasvector2d(geo->getEndPoint(),geo->getLastParameter());
        }
        else if (isperiodicconic || isbounded)
        {
            auto geo = static_cast<const Part::GeomConic*>(geometry);
            double segment = (geo->getLastParameter() - geo->getFirstParameter())
                / curvedEdgeCountSegments;
            for (int i = 0; i < curvedEdgeCountSegments; i++) {
                double u = geo->getFirstParameter() + i * segment;
                emplaceasvector2d(geo->value(u),u);
            }
            // either close the curve for untrimmed conic or set the last point for bounded curves
			double u = isperiodicconic ? 0 : geo->getLastParameter();
            emplaceasvector2d( geo->value(u) ,u);
        }
    }
}