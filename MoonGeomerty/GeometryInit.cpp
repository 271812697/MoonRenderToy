#include "Geometry.h"
#include "GeometryInit.h"
#include "base/BaseClass.h"
#include "base/Exception.h"
#include "App/FaceMaker.h"
#include "App/FaceMakerBullseye.h"
#include "App/FaceMakerCheese.h"
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
		GeomBSplineCurve::init();
		GeomTrimmedCurve::init();
		GeomArcOfConic::init();
		GeomArcOfCircle::init();
		GeomLineSegment::init();
		GeomCircle::init();
		GeomEllipse::init();
		GeomHyperbola::init();
		GeomParabola::init();

		FaceMaker::init();
		FaceMakerPublic::init();
		FaceMakerSimple::init();
		FaceMakerBullseye::init();
		FaceMakerCheese::init();
	}
}