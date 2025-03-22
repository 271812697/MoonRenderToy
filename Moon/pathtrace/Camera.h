
#pragma once

#include <float.h>
#include"MathUtil.h"

namespace PathTrace
{
    class Camera
    {
    public:
        Camera(Vec3 eye, Vec3 lookat, float fov);
        Camera(const Camera& other);
        Camera& operator = (const Camera& other);

        void OffsetOrientation(float dx, float dy);
        void Strafe(float dx, float dy);
        void SetRadius(float dr);
        void ComputeViewProjectionMatrix(float* view, float* projection, float ratio);
        void SetFov(float val);
        //相机采用右手坐标系，up right forward表示的是左手坐标系

        Vec3 position;
        Vec3 up;
        Vec3 right;
        Vec3 forward;

        float focalDist;
        float aperture;
        float fov;
        Vec3 pivot;
        bool isMoving;

    private:
        void UpdateCamera();

        Vec3 worldUp;
        

        float pitch;
        float radius;
        float yaw;
    };
}