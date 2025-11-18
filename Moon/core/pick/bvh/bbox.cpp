#include "bbox.h"
namespace MOON
{
	FVector3 bbox::center()  const { return (pmax + pmin) * 0.5f; }
	FVector3 bbox::extents() const { return pmax - pmin; }
	//包围盒面积
	float bbox::surface_area() const
	{
		FVector3 ext = extents();
		return 2.f * (ext.x * ext.y + ext.x * ext.z + ext.y * ext.z);
	}

	// Grow the bounding box by a point
	void bbox::grow(FVector3 const& p)
	{
		pmin = FVector3::Min(pmin, p);
		pmax = FVector3::Max(pmax, p);
	}
	// Grow the bounding box by a box
	void bbox::grow(bbox const& b)
	{
		pmin = FVector3::Min(pmin, b.pmin);
		pmax = FVector3::Max(pmax, b.pmax);
	}
	bool bbox::contains(FVector3 const& p) const
	{
		FVector3 radius = extents() * 0.5f;
		return std::abs(center().x - p.x) <= radius.x &&
			fabs(center().y - p.y) <= radius.y &&
			fabs(center().z - p.z) <= radius.z;
	}
	//两个包围盒求并
	bbox bboxunion(bbox const& box1, bbox const& box2)
	{
		bbox res;
		res.pmin = FVector3::Min(box1.pmin, box2.pmin);
		res.pmax = FVector3::Max(box1.pmax, box2.pmax);
		return res;
	}
	//两个包围盒求交
	bbox intersection(bbox const& box1, bbox const& box2)
	{
		return bbox(FVector3::Max(box1.pmin, box2.pmin), FVector3::Min(box1.pmax, box2.pmax));
	}
	void intersection(bbox const& box1, bbox const& box2, bbox& box)
	{
		box.pmin = FVector3::Max(box1.pmin, box2.pmin);
		box.pmax = FVector3::Min(box1.pmax, box2.pmax);
	}
#define BBOX_INTERSECTION_EPS 0.f
	bool intersects(bbox const& box1, bbox const& box2)
	{
		FVector3 b1c = box1.center();
		FVector3 b1r = box1.extents() * 0.5f;
		FVector3 b2c = box2.center();
		FVector3 b2r = box2.extents() * 0.5f;

		return (fabs(b2c.x - b1c.x) - (b1r.x + b2r.x)) <= BBOX_INTERSECTION_EPS &&
			(fabs(b2c.y - b1c.y) - (b1r.y + b2r.y)) <= BBOX_INTERSECTION_EPS &&
			(fabs(b2c.z - b1c.z) - (b1r.z + b2r.z)) <= BBOX_INTERSECTION_EPS;
	}
	bool contains(bbox const& box1, bbox const& box2)
	{
		return box1.contains(box2.pmin) && box1.contains(box2.pmax);
	}
}
