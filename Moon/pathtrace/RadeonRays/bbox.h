#pragma once

#ifndef BBOX_H
#define BBOX_H

#include <cmath>
#include <algorithm>
#include <limits>
#include "../MathUtil.h"

using namespace PathTrace;

namespace RadeonRays
{
    class bbox
    {
    public:
        bbox()
            : pmin(Vec3(std::numeric_limits<float>::max(),
                        std::numeric_limits<float>::max(),
                        std::numeric_limits<float>::max()))
            , pmax(Vec3(-std::numeric_limits<float>::max(),
                        -std::numeric_limits<float>::max(),
                        -std::numeric_limits<float>::max()))
        {
        }

        bbox(Vec3 const& p)
            : pmin(p)
            , pmax(p)
        {
        }

        bbox(Vec3 const& p1, Vec3 const& p2)
            : pmin(Vec3::Min(p1, p2))
            , pmax(Vec3::Max(p1, p2))
        {
        }

		Vec3 center()  const;
		Vec3 extents() const;
        float diagonalDistance()const;

        bool contains(Vec3 const& p) const;

		inline int maxdim() const
		{
			Vec3 ext = extents();

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
        Vec3 const& operator [] (int i) const { return *(&pmin + i); }

        // Grow the bounding box by a point
		void grow(Vec3 const& p);
        // Grow the bounding box by a box
		void grow(bbox const& b);

        Vec3 pmin;
        Vec3 pmax;
    };

	bbox bboxunion(bbox const& box1, bbox const& box2);
	bbox intersection(bbox const& box1, bbox const& box2);
	void intersection(bbox const& box1, bbox const& box2, bbox& box);
	bool intersects(bbox const& box1, bbox const& box2);
	bool contains(bbox const& box1, bbox const& box2);
}

#endif