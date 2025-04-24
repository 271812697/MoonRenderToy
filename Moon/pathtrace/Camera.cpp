#include <iostream>
#include <cstring>
#include "Camera.h"

namespace PathTrace
{

	//采用右手坐标系，存的时候转置一下
	void Frustum(float left, float right, float bottom, float top, float znear, float zfar, float* m16)
	{
		float temp, temp2, temp3, temp4;
		temp = 2.0f * znear;
		temp2 = right - left;
		temp3 = top - bottom;
		temp4 = zfar - znear;
		m16[0] = temp / temp2;
		m16[1] = 0.0;
		m16[2] = 0.0;
		m16[3] = 0.0;
		m16[4] = 0.0;
		m16[5] = temp / temp3;
		m16[6] = 0.0;
		m16[7] = 0.0;
		m16[8] = (right + left) / temp2;
		m16[9] = (top + bottom) / temp3;
		m16[10] = (-zfar - znear) / temp4;
		m16[11] = -1.0f;
		m16[12] = 0.0;
		m16[13] = 0.0;
		m16[14] = (-temp * zfar) / temp4;
		m16[15] = 0.0;
	}

	void Perspective(float fovyInDegrees, float aspectRatio, float znear, float zfar, float* m16)
	{
		float ymax, xmax;
		ymax = znear * tanf(fovyInDegrees * 3.141592f / 180.0f);
		xmax = ymax * aspectRatio;
		Frustum(-xmax, xmax, -ymax, ymax, znear, zfar, m16);
	}

	void Cross(const float* a, const float* b, float* r)
	{
		r[0] = a[1] * b[2] - a[2] * b[1];
		r[1] = a[2] * b[0] - a[0] * b[2];
		r[2] = a[0] * b[1] - a[1] * b[0];
	}

	float Dot(const float* a, const float* b)
	{
		return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
	}

	void Normalize(const float* a, float* r)
	{
		float il = 1.f / (sqrtf(Dot(a, a)) + FLT_EPSILON);
		r[0] = a[0] * il;
		r[1] = a[1] * il;
		r[2] = a[2] * il;
	}

	void LookAt(const float* eye, const float* at, const float* up, float* m16)
	{
		float X[3], Y[3], Z[3], tmp[3];

		tmp[0] = eye[0] - at[0];
		tmp[1] = eye[1] - at[1];
		tmp[2] = eye[2] - at[2];

		Normalize(tmp, Z);
		Normalize(up, Y);

		Cross(Y, Z, tmp);
		Normalize(tmp, X);

		Cross(Z, X, tmp);
		Normalize(tmp, Y);

		m16[0] = X[0];
		m16[1] = Y[0];
		m16[2] = Z[0];
		m16[3] = 0.0f;
		m16[4] = X[1];
		m16[5] = Y[1];
		m16[6] = Z[1];
		m16[7] = 0.0f;
		m16[8] = X[2];
		m16[9] = Y[2];
		m16[10] = Z[2];
		m16[11] = 0.0f;
		m16[12] = -Dot(X, eye);
		m16[13] = -Dot(Y, eye);
		m16[14] = -Dot(Z, eye);
		m16[15] = 1.0f;
	}

	Camera::Camera(Vec3 eye, Vec3 lookat, float fov)
	{
		position = eye;
		pivot = lookat;
		radius = Vec3::Distance(eye, lookat);
		worldUp = Vec3(0, 1, 0);
		this->fov = Math::Radians(fov);
		focalDist = 0.1f;
		aperture = 0.0;
		float m[16];
		LookAt(&eye.x, &pivot.x, &worldUp.x, m);
		forward = Vec3::Normalize(pivot - position);
		up = { m[1],m[5],m[9] };
		right = { m[0],m[4],m[8] };
		UpdateCamera();
		std::cout << Vec3::Length(forward) << ":" << Vec3::Length(up) << ":" << Vec3::Length(right) << std::endl;;

	}

	Camera::Camera(const Camera& other)
	{
		*this = other;
	}

	Camera& Camera::operator = (const Camera& other)
	{
		ptrdiff_t l = (unsigned char*)&isMoving - (unsigned char*)&position.x;
		isMoving = memcmp(&position.x, &other.position.x, l) != 0;
		memcpy(&position.x, &other.position.x, l);
		return *this;
	}


	void Camera::OffsetRotateByScreen(float dx, float dy)
	{
		Vec3 axis = dx * lastright + dy * lastup;
		float len = Vec3::Length(axis);
		if (len < FLT_EPSILON) {
			return;
		}

		Vec3 rot = -Vec3::Normalize(-dy * lastright + dx * lastup);
		position = RoatePoint(lastposition, rot, pivot, len / 500);
		forward = Vec3::Normalize(pivot - position);
		up = RoateDir(lastup, rot, len / 500);
		right = RoateDir(lastright, rot, len / 500);
		std::cout << Vec3::Length(forward) << ":" << Vec3::Length(up) << ":" << Vec3::Length(right) << std::endl;;


	}

	void Camera::setPivot(const Vec3& p)
	{
		pivot = p;
		position = pivot - radius * forward;
	}

	void Camera::Strafe(float dx, float dy)
	{
		Vec3 translation = right * -dx + up * dy;
		pivot = pivot + translation;
		position = pivot - radius * forward;
	}

	void Camera::SetRadius(float dr)
	{
		radius += dr;
		position = pivot - radius * forward;
	}

	void Camera::SetFov(float val)
	{
		fov = Math::Radians(val);
	}

	void Camera::UpdateCamera()
	{
		lastposition = position;
		lastup = up;
		lastright = right;
		lastforward = forward;
	}

	void Camera::ComputeViewProjectionMatrix(float* view, float* projection, float ratio)
	{
		Vec3 at = position + forward;
		LookAt(&position.x, &at.x, &up.x, view);
		const float fov_v = (1.f / ratio) * tanf(fov / 2.f);
		Perspective(Math::Degrees(fov_v), ratio, 0.1f, 1000.f, projection);
	}
}
