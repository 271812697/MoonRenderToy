#include "Geomerty/TopoShape.h"
namespace MOON {
	TopoShape::TopoShape(const TopoDS_Shape& shape):_Shape(*this, shape)
	{
	}
	TopoShape::~TopoShape()
	{
	}
	void TopoShape::getPoints(std::vector<Maths::FVector3>& Points, std::vector<Maths::FVector3>& Normals, double Accuracy, uint16_t flags) const
	{
	}
	void TopoShape::getFaces(std::vector<Maths::FVector3>& Points, std::vector<Facet>& faces, double Accuracy, uint16_t flags) const
	{
	}
	void TopoShape::setFaces(const std::vector<Maths::FVector3>& Points, const std::vector<Facet>& faces, double tolerance)
	{
	}
	void TopoShape::getDomains(std::vector<Domain>&) const
	{
	}
	TopoDS_Shape& TopoShape::move(TopoDS_Shape& tds, const TopLoc_Location& location)
	{
#if OCC_VERSION_HEX < 0x070600
		tds.Move(location);
#else
		tds.Move(location, false);
#endif
		return tds;
	}
	TopoDS_Shape& TopoShape::locate(TopoDS_Shape& tds, const TopLoc_Location& loc)
	{
		tds.Location(TopLoc_Location());
		return move(tds, loc);
	}
}