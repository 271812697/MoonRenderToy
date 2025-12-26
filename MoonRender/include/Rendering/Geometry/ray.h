#pragma once
#include "Maths/FVector3.h"
#include "Maths/FMatrix4.h"
#include "bbox.h"


namespace Rendering::Geometry
{
class  Ray
{
public:
    /// Construct a degenerate ray with zero origin and direction.
    Ray() noexcept = default;

    /// Construct from origin and direction. The direction will be normalized.
    Ray(const Maths::FVector3& origin, const Maths::FVector3& direction) noexcept
    {
        Define(origin, direction);
    }

    /// Copy-construct from another ray.
    Ray(const Ray& ray) noexcept = default;

    /// Assign from another ray.
    Ray& operator =(const Ray& rhs) noexcept = default;



    /// Define from origin and direction. The direction will be normalized.
    void Define(const Maths::FVector3& origin, const Maths::FVector3& direction)
    {
        origin_ = origin;
        direction_ = Maths::FVector3::Normalize(direction);
    }
    Maths::FVector3 Value(float t) {
        return origin_ + direction_ * t;
    }
    /// Project a point on the ray.
    Maths::FVector3 Project(const Maths::FVector3& point) const
    {
        Maths::FVector3 offset = point - origin_;
        return origin_ + direction_*Maths::FVector3::Dot(offset,direction_) ;
    }

    /// Return distance of a point from the ray.
    float Distance(const Maths::FVector3& point) const
    {
        Maths::FVector3 projected = Project(point);
        return (point - projected).Length();
    }

    /// Return closest point to another ray.
    Maths::FVector3 ClosestPoint(const Ray& ray) const;
 
    /// Return hit distance to a bounding box, or infinity if no hit.
    bool HitDistance(const bbox& box,float& outDist) const;
    /// Return hit distance to a triangle, or infinity if no hit. Optionally return hit normal and hit barycentric coordinate at intersect point.
    bool HitDistance(const Maths::FVector3& v0, const Maths::FVector3& v1, const Maths::FVector3& v2,float& outDist ,Maths::FVector3* outNormal = nullptr, Maths::FVector3* outBary = nullptr) ;
    /// Return hit distance to non-indexed geometry data, or infinity if no hit. Optionally return hit normal and hit uv coordinates at intersect point.
  
  
    /// Return transformed by a 3x4 matrix. This may result in a non-normalized direction.
    Ray Transformed(const Maths::FMatrix4& transform) const;

    /// Ray origin.
    Maths::FVector3 origin_;
    /// Ray direction.
    Maths::FVector3 direction_;
};

}
