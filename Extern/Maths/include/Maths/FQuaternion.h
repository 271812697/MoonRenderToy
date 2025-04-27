#pragma once

#include "Maths/FVector3.h"
#include "Maths/FMatrix3.h"
#include "Maths/FMatrix4.h"

#include <string>

namespace Maths
{
	struct FQuaternion
	{
	public:
		float x;
		float y;
		float z;
		float w;

		static const FQuaternion Identity;

		FQuaternion();

		FQuaternion(float p_real);

		FQuaternion(float p_x, float p_y, float p_z, float p_w);

		FQuaternion(const FQuaternion& p_other);

		FQuaternion(const FMatrix3& p_rotationMatrix);

		FQuaternion(const FMatrix4& p_rotationMatrix);

		FQuaternion(const FVector3& p_euler);

		static FQuaternion LookAt(const FVector3& p_forward, const FVector3& p_up);

		static bool IsIdentity(const FQuaternion& p_target);

		static bool IsPure(const FQuaternion& p_target);

		static bool IsNormalized(const FQuaternion& p_target);

		static float DotProduct(const FQuaternion& p_left, const FQuaternion& p_right);	

		static FQuaternion Normalize(const FQuaternion& p_target);

		static float Length(const FQuaternion& p_target);

		static float LengthSquare(const FQuaternion& p_target);

		static float GetAngle(const FQuaternion& p_target);

		static FVector3 GetRotationAxis(const FQuaternion& p_target);

		static FQuaternion Inverse(const FQuaternion& p_target);

		static FQuaternion Conjugate(const FQuaternion& p_target);

		static FQuaternion Square(const FQuaternion& p_target);

		static std::pair<FVector3, float> GetAxisAndAngle(const FQuaternion& p_target);

		static float AngularDistance(const FQuaternion& p_left, const FQuaternion& p_right);

		static FQuaternion Lerp(const FQuaternion& p_start, const FQuaternion& p_end, float p_alpha);

		static FQuaternion Slerp(const FQuaternion& p_start, const FQuaternion& p_end, float p_alpha);

		static FQuaternion Nlerp(const FQuaternion& p_start, const FQuaternion& p_end, float p_alpha);

		static FVector3 RotatePoint(const FVector3& p_point, const FQuaternion& p_quaternion);

		static FVector3 RotatePoint(const FVector3& p_point, const FQuaternion& p_quaternion, const FVector3& p_pivot);

		static FVector3 EulerAngles(const FQuaternion& p_target);

		static FMatrix3 ToMatrix3(const FQuaternion& p_target);

		static FMatrix4 ToMatrix4(const FQuaternion& p_target);

		bool operator==(const FQuaternion& p_otherQuat) const;
		bool operator!=(const FQuaternion& p_otherQuat) const;
		FQuaternion operator+(const FQuaternion& p_otherQuat) const;
		FQuaternion& operator+=(const FQuaternion& p_otherQuat);
		FQuaternion operator-(const FQuaternion& p_otherQuat) const;
		FQuaternion& operator-=(const FQuaternion& p_otherQuat);
		float operator|(const FQuaternion& p_otherQuat) const;
		FQuaternion& operator*=(const float p_scale);
		FQuaternion operator*(const float p_scale) const;
		FQuaternion operator*(const FQuaternion& p_otherQuat) const;
		FQuaternion& operator*=(const FQuaternion& p_otherQuat);
		FVector3 operator*(const FVector3& p_toMultiply) const;
		FMatrix3 operator*(const FMatrix3& p_multiply) const;
		FQuaternion& operator/=(const float p_scale);
		FQuaternion operator/(const float p_scale) const;
	};
}