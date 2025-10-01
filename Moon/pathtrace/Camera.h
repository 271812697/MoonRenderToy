#pragma once
#include <float.h>
#include <vector>
#include"MathUtil.h"

namespace PathTrace
{
	class Renderer;
	class Camera
	{
	public:
		Camera(Vec3 eye, Vec3 lookat, float fov);
		Camera(const Camera& other);
		Camera& operator = (const Camera& other);
		void OffsetRotateByScreen(float dx, float dy);
		void setPivot(const Vec3& p);
		void strafe(float dx, float dy);
		//void update();
		void setRadius(float r);
		void offsetRadius(float dr);
		void computeViewProjectionMatrix(float* view, float* projection, float ratio);
		void setFov(float val);
		float getFov();
		float getFocalDist();
		void setFocalDist(float val);
		float getAperture();
		void setAperture(float val);
		Vec3 getUp();
		Vec3 getForward();
		Vec3 getRight();
		Vec3 getEye();
		Vec3& getPivoit();
		Vec3 getPositon();
		void setPosition(const Vec3& pos);
		void lookAt(const Vec3& eye, const Vec3& lookat);
		//相机采用右手坐标系，up right forward表示的是左手坐标系
		void updateCamera();
	private:
		friend Renderer;
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
		Vec3 worldUp;
		float radius;
	};
}