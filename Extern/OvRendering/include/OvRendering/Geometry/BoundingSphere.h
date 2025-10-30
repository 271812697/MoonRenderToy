

#pragma once

#include <OvMaths/FVector3.h>

namespace OvRendering::Geometry
{
	/**
	* Data structure that defines a bounding sphere (Position + radius)
	*/
	struct BoundingSphere
	{
		OvMaths::FVector3 position;
		float radius;
		void merge(const BoundingSphere& other)
		{
			OvMaths::FVector3 centerDir = other.position - position;
			float distance = OvMaths::FVector3::Length(centerDir);
			// If one sphere is completely inside the other, take the larger one
			if (distance + other.radius <= radius)
			{
				return;
			}
			if (distance + radius <= other.radius)
			{
				position = other.position;
				radius = other.radius;
				return;
			}
			// Otherwise, calculate the new sphere that encompasses both
			float newRadius = (distance + radius + other.radius) * 0.5f;
			OvMaths::FVector3 newCenter = position;
			if (distance > 0.0f)
			{
				newCenter += centerDir * ((newRadius - radius) / distance);
			}
			position = newCenter;
			radius = newRadius;
		}
	};
}