
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

		void OffsetRotateByScreen(float dx, float dy);
		void setPivot(const Vec3& p);
		void Strafe(float dx, float dy);
		void Update();
		void SetRadius(float dr);
		void ComputeViewProjectionMatrix(float* view, float* projection, float ratio);
		void SetFov(float val);
		Vec3 GetEye();
		//相机采用右手坐标系，up right forward表示的是左手坐标系
		Vec3 position;
		Vec3 up;
		Vec3 right;
		Vec3 forward;
		Vec3 pivot;
		Vec3 lastposition;
		Vec3 lastup;
		Vec3 lastright;
		Vec3 lastforward;
		Vec3 lastpivot;
		

		float focalDist;
		float aperture;
		float fov;

		bool isMoving;
		void UpdateCamera();
	private:

		Vec3 worldUp;
		float radius;
	};
}