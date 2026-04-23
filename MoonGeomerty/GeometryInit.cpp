#include "Geometry.h"
#include "GeometryInit.h"
#include "base/BaseClass.h"
#include "base/Exception.h"
namespace Part {
	void Part::GeometryTypeInit()
	{
		Base::Type::init();;
		Base::BaseClass::init();
		Base::Exception::init();
		Base::AbortException::init();
		Base::Persistence::init();
		// parent class must be initialized before subclass
		Geometry::init();
		GeomPoint::init();
		GeomCurve::init();
		GeomConic::init();
		GeomLine::init();
		GeomBoundedCurve::init();
		GeomTrimmedCurve::init();
		GeomArcOfConic::init();
		GeomArcOfCircle::init();
		GeomLineSegment::init();
		GeomCircle::init();
		GeomEllipse::init();
		GeomHyperbola::init();
		GeomParabola::init();
	}
}