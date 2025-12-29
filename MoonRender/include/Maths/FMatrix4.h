#pragma once
#include <stdint.h>
#include <Maths/FMatrix3.h>
#include <Maths/FVector3.h>
#include <Maths/FVector4.h>

namespace Maths
{
	struct FQuaternion;
	struct FMatrix4
	{
		float data[16];
		static const FMatrix4 Identity;
		FMatrix4();
		FMatrix4(float p_element1, float p_element2, float p_element3,
			float p_element4, float p_element5, float p_element6,
			float p_element7, float p_element8, float p_element9,
			float p_element10, float p_element11, float p_element12,
			float p_element13, float p_element14, float p_element15, float p_element16);
		FMatrix4(const FMatrix4& p_other);
		FMatrix4(const FMatrix3& p_other);
		FMatrix4& operator=(const FMatrix4& p_other);
		bool operator==(const FMatrix4& p_other);
		FMatrix4 operator+(const FMatrix4& p_other) const;
		FMatrix4& operator+=(const FMatrix4& p_other);
		FMatrix4 operator-(float p_scalar) const;
		FMatrix4 operator-(const FMatrix4& p_other) const;
		FMatrix4& operator-=(const FMatrix4& p_other);
		FMatrix4 operator*(float p_scalar) const;
		FMatrix4& operator*=(float p_scalar);
		FVector4 operator*(const FVector4& p_vector) const;
		FMatrix4 operator*(const FMatrix4& p_other) const;
		FMatrix4& operator*=(const FMatrix4& p_other);
		FMatrix4 operator/(float p_scalar) const;
		FMatrix4& operator/=(float p_scalar);
		FMatrix4 operator/(const FMatrix4& p_other) const;
		FMatrix4& operator/=(const FMatrix4& p_other);
		float& operator()(uint8_t p_row, uint8_t p_column);
		float operator()(uint8_t p_row, uint8_t p_column)const;
		FMatrix4 RotateOnAxisY(float p_rotation)const;
		FMatrix4 RotateOnAxisX(float p_rotation)const;
		FMatrix4 RotateOnAxisZ(float p_rotation)const;
		static bool AreEquals(const FMatrix4& p_left, const FMatrix4& p_right);
		static bool AreAlmostEquals(const FMatrix4& p_left, const FMatrix4& p_right);
		static FMatrix4 Add(const FMatrix4& p_left, float p_scalar);
		static FMatrix4 Add(const FMatrix4& p_left, const FMatrix4& p_right);
		static FMatrix4 Subtract(const FMatrix4& p_left, float p_scalar);
		static FMatrix4 Subtract(const FMatrix4& p_left, const FMatrix4& p_right);
		static FMatrix4 Multiply(const FMatrix4& p_left, float p_scalar);
		static FVector4 Multiply(const FMatrix4& p_matrix, const FVector4& p_vector);
		static FMatrix4 Multiply(const FMatrix4& p_left, const FMatrix4& p_right);
		static FMatrix4 Divide(const FMatrix4& p_left, float p_scalar);
		static FMatrix4 Divide(const FMatrix4& p_left, const FMatrix4& p_right);
		static bool IsIdentity(const FMatrix4& p_matrix);
		static float GetMinor(float p_minor0, float p_minor1, float p_minor2,
			float p_minor3, float p_minor4, float p_minor5,
			float p_minor6, float p_minor7, float p_minor8);
		static float Determinant(const FMatrix4& p_matrix);
		static FMatrix4 Transpose(const FMatrix4& p_matrix);
		FMatrix4 TransposeMartix();
		static FMatrix4 Inverse(const FMatrix4& p_matrix);
		FMatrix4 Inverse();
		static FMatrix4 Translation(const FVector3& p_translation);
		static FVector3 MulPoint(const FMatrix4& p_matrix, const FVector3& p);
		static FVector3 MulDir(const FMatrix4& p_matrix, const FVector3& p);
		FVector3 MulPoint(const FVector3& p)const;
		FVector3 MulDir(const FVector3& p)const ;
		FVector3 MulDirNotNormlaize(const FVector3& p)const;
		static FMatrix4 Translate(const FMatrix4& p_matrix, const FVector3& p_translation);
		static FMatrix4 RotationOnAxisX(float p_rotation);
		static FMatrix4 RotateOnAxisX(const FMatrix4& p_matrix, float p_rotation);
		static FMatrix4 RotationOnAxisY(float p_rotation);
		static FMatrix4 RotateOnAxisY(const FMatrix4& p_matrix, float p_rotation);
		static FMatrix4 RotationOnAxisZ(float p_rotation);
		static FMatrix4 RotateOnAxisZ(const FMatrix4& p_matrix, float p_rotation);
		static FMatrix4 RotationYXZ(float p_rotation);
		static FMatrix4 RotateYXZ(const FMatrix4& p_matrix, float p_rotation);
		static FMatrix4 Scaling(const FVector3& p_scale);
		static FMatrix4 Scale(const FMatrix4& p_matrix, const FVector3& p_scale);
		static FMatrix4 Rotation(const FQuaternion& p_quaternion);
		static FMatrix4 Rotate(const FMatrix4& p_matrix, const FQuaternion& p_quaternion);
		static FMatrix4 CreatePerspective(const float p_fov, const float p_aspectRatio, const float p_zNear, const float p_zFar);
		static FMatrix4 CreateOrthographic(const float p_size, const float p_aspectRatio, const float p_zNear, const float p_zFar);
		static FMatrix4 CreateView(const float p_eyeX, const float p_eyeY, const float p_eyeZ, const float p_lookX, const float p_lookY, const float p_lookZ, const float p_upX, const float p_upY, const float p_upZ);
		static FMatrix4 CreateCameraView(const FVector3& eye, const FVector3& at, const FVector3& up);
		static FMatrix4 CreateFrustum(const float p_left, const float p_right, const float p_bottom, const float p_top, const float p_zNear, const float p_zFar);
		static FVector4 GetRow(const FMatrix4& p_matrix, uint8_t p_row);
		static FVector4 GetColumn(const FMatrix4& p_matrix, uint8_t p_column);
	};
}