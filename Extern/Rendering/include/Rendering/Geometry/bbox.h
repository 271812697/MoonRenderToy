#pragma once
#include <cmath>
#include <algorithm>
#include <limits>
#include "Maths/FVector3.h"
namespace Rendering::Geometry
{
	using namespace Maths;
	class bbox
	{
	public:
		bbox()
			: pmin(FVector3(std::numeric_limits<float>::max(),
				std::numeric_limits<float>::max(),
				std::numeric_limits<float>::max()))
			, pmax(FVector3(-std::numeric_limits<float>::max(),
				-std::numeric_limits<float>::max(),
				-std::numeric_limits<float>::max()))
		{
		}

		bbox(FVector3 const& p)
			: pmin(p)
			, pmax(p)
		{
		}

		bbox(FVector3 const& p1, FVector3 const& p2)
			: pmin(FVector3::Min(p1, p2))
			, pmax(FVector3::Max(p1, p2))
		{
		}

		FVector3 center()  const;
		FVector3 extents() const;

		bool contains(FVector3 const& p) const;

		inline int maxdim() const
		{
			FVector3 ext = extents();

			if (ext.x >= ext.y && ext.x >= ext.z)
				return 0;
			if (ext.y >= ext.x && ext.y >= ext.z)
				return 1;
			if (ext.z >= ext.x && ext.z >= ext.y)
				return 2;

			return 0;
		}

		float surface_area() const;

		// TODO: this is non-portable, optimization trial for fast intersection test
		FVector3 const& operator [] (int i) const { return *(&pmin + i); }

		// Grow the bounding box by a point
		void grow(FVector3 const& p);
		// Grow the bounding box by a box
		void grow(bbox const& b);

		FVector3 pmin;
		FVector3 pmax;
	};

	bbox bboxunion(bbox const& box1, bbox const& box2);
	bbox intersection(bbox const& box1, bbox const& box2);
	void intersection(bbox const& box1, bbox const& box2, bbox& box);
	bool intersects(bbox const& box1, bbox const& box2);
	bool contains(bbox const& box1, bbox const& box2);
}

