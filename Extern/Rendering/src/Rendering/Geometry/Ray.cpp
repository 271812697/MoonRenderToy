#include "Rendering/Geometry/ray.h"

namespace Rendering::Geometry
{
static  float M_INFINITY = 1e9;
Maths::FVector3 Ray::ClosestPoint(const Ray& ray) const
{
    // Algorithm based on http://paulbourke.net/geometry/lineline3d/
    Maths::FVector3 p13 = origin_ - ray.origin_;
    Maths::FVector3 p43 = ray.direction_;
    Maths::FVector3 p21 = direction_;

    float d1343 = p13.Dot(p43);
    float d4321 = p43.Dot(p21);
    float d1321 = p13.Dot(p21);
    float d4343 = p43.Dot(p43);
    float d2121 = p21.Dot(p21);

    float d = d2121 * d4343 - d4321 * d4321;
    if (abs(d) < 1e-6)
        return origin_;
    float n = d1343 * d4321 - d1321 * d4343;
    float a = n / d;

    return origin_ + a * direction_;
}


bool Ray::HitDistance(const bbox& box,float& outDis) const
{
    // If undefined, no hit (infinite distance)
    if (!box.isValid())
        return false;

    // Check for ray origin being inside the box
    if (box.contains(origin_))
    {
        outDis = 0.0f;
        return true;
    }
        
    bool flag = false;

    float dist = M_INFINITY;

    // Check for intersecting in the X-direction
    if (origin_.x < box.pmin.x && direction_.x > 0.0f)
    {
        float x = (box.pmin.x - origin_.x) / direction_.x;
        if (x < dist)
        {
            Maths::FVector3 point = origin_ + x * direction_;
            if (point.y >= box.pmin.y && point.y <= box.pmax.y && point.z >= box.pmin.z && point.z <= box.pmax.z)
            {
                flag = true;
                dist = x;
            }
               
        }
    }
    if (origin_.x > box.pmax.x && direction_.x < 0.0f)
    {
        float x = (box.pmax.x - origin_.x) / direction_.x;
        if (x < dist)
        {
            Maths::FVector3 point = origin_ + x * direction_;
            if (point.y >= box.pmin.y && point.y <= box.pmax.y && point.z >= box.pmin.z && point.z <= box.pmax.z)
            {
                flag = true;
                dist = x;
            }  
        }
    }
    // Check for intersecting in the Y-direction
    if (origin_.y < box.pmin.y && direction_.y > 0.0f)
    {
        float x = (box.pmin.y - origin_.y) / direction_.y;
        if (x < dist)
        {
            Maths::FVector3 point = origin_ + x * direction_;
            if (point.x >= box.pmin.x && point.x <= box.pmax.x && point.z >= box.pmin.z && point.z <= box.pmax.z)
            { 
                dist = x;
                flag = true;
            } 
        }
    }
    if (origin_.y > box.pmax.y && direction_.y < 0.0f)
    {
        float x = (box.pmax.y - origin_.y) / direction_.y;
        if (x < dist)
        {
            Maths::FVector3 point = origin_ + x * direction_;
            if (point.x >= box.pmin.x && point.x<= box.pmax.x && point.z >= box.pmin.z && point.z <= box.pmax.z)
            {
                flag = true;
                dist = x;
            }
        }
    }
    // Check for intersecting in the Z-direction
    if (origin_.z < box.pmin.z && direction_.z > 0.0f)
    {
        float x = (box.pmin.z - origin_.z) / direction_.z;
        if (x < dist)
        {
            Maths::FVector3 point = origin_ + x * direction_;
            if (point.x >= box.pmin.x && point.x <= box.pmax.x && point.y >= box.pmin.y && point.y <= box.pmax.y)
            {
                flag = true;
                dist = x;
            }
        }
    }
    if (origin_.z > box.pmax.z && direction_.z < 0.0f)
    {
        float x = (box.pmax.z - origin_.z) / direction_.z;
        if (x < dist)
        {
            Maths::FVector3 point = origin_ + x * direction_;
            if (point.x >= box.pmin.x && point.x <= box.pmax.x && point.y >= box.pmin.y && point.y <= box.pmax.y)
            {
                flag = true;
                dist = x;
            }
        }
    }
    outDis = dist;
    return flag;
}


bool Ray::HitDistance(const Maths::FVector3& v0, const Maths::FVector3& v1, const Maths::FVector3& v2, float& outDist,Maths::FVector3* outNormal, Maths::FVector3* outBary)
{
    // Based on Fast, Minimum Storage Ray/Triangle Intersection by Möller & Trumbore
     // http://www.graphics.cornell.edu/pubs/1997/MT97.pdf
     // Calculate edge vectors
    Maths::FVector3 edge1(v1 - v0);
    Maths::FVector3 edge2(v2 - v0);
   

    // Calculate determinant & check backfacing
    Maths::FVector3 p(direction_.Cross(edge2));
    float det = edge1.Dot(p);
    if (abs(det) >= 1e-6)
    {
        // Calculate u & v parameters and test
        Maths::FVector3 t(origin_ - v0);
        float u = t.Dot(p)/det;
        if (u >= 0.0f && u <= 1.0f)
        {
            Maths::FVector3 q(t.Cross(edge1));
            float v = direction_.Dot(q)/det;
            if (v >= 0.0f && u + v <= 1.0f)
            {
                float distance = edge2.Dot(q) / det;
                // Discard hits behind the ray
                if (distance >= 0.0f)
                {
                    // There is an intersection, so calculate distance & optional normal
                    if (outNormal)
                        *outNormal = edge1.Cross(edge2);
                    if (outBary)
                        *outBary = Maths::FVector3(1 - (u ) - (v ), u , v );
                    outDist = distance;
                    return true;
                }
            }
        }
    }
    return false;
}




Ray Ray::Transformed(const Maths::FMatrix4& transform) const
{
    Ray ret;
    ret.origin_ = transform.MulPoint(origin_);
    ret.direction_ = transform.MulDirNotNormlaize(direction_);
    
    return ret;
}

}
