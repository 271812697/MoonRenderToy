#pragma once
#include <stdint.h>
#include "Maths/FVector3.h"
#include "Maths/FVector2.h"

namespace Maths
{

	struct FMatrix3
	{
		float data[9];
		static const FMatrix3 Identity;

	
		FMatrix3();

		FMatrix3(float p_value);

	
		FMatrix3(float p_element1, float p_element2, float p_element3,
				 float p_element4, float p_element5, float p_element6, 
				 float p_element7, float p_element8, float p_element9);


		FMatrix3(const FMatrix3& p_other);

	
		FMatrix3& operator=(const FMatrix3& p_other);


		bool operator==(const FMatrix3& p_other);

	
		FMatrix3 operator+(const FMatrix3& p_other) const;

		FMatrix3& operator+=(const FMatrix3& p_other);


		FMatrix3 operator-(const FMatrix3& p_other) const;


		FMatrix3& operator-=(const FMatrix3& p_other);

		FMatrix3 operator*(float p_scalar) const;


		FMatrix3& operator*=(float p_scalar);


		FVector3 operator*(const FVector3& p_vector) const;

		FMatrix3 operator*(const FMatrix3& p_other) const;

		FMatrix3& operator*=(const FMatrix3& p_other);

		FMatrix3 operator/(float p_scalar) const;

		FMatrix3& operator/=(float p_scalar);

		FMatrix3 operator/(const FMatrix3& p_other) const;

		FMatrix3& operator/=(const FMatrix3& p_other);

		float& operator()(uint8_t p_row, uint8_t p_column);

		static bool AreEquals(const FMatrix3& p_left, const FMatrix3& p_right);

		static FMatrix3 Add(const FMatrix3& p_left, float p_scalar);

		static FMatrix3 Add(const FMatrix3& p_left, const FMatrix3& p_right);

		static FMatrix3 Subtract(const FMatrix3& p_left, float p_scalar);

		static FMatrix3 Subtract(const FMatrix3& p_left, const FMatrix3& p_right);

		static FMatrix3 Multiply(const FMatrix3& p_left, float p_scalar);

		static FVector3 Multiply(const FMatrix3& p_matrix, const FVector3& p_vector);


		static FMatrix3 Multiply(const FMatrix3& p_left, const FMatrix3& p_right);

		static FMatrix3 Divide(const FMatrix3& p_left, float p_scalar);

		static FMatrix3 Divide(const FMatrix3& p_left, const FMatrix3& p_right);

		static bool IsIdentity(const FMatrix3& p_matrix);

		static float Determinant(const FMatrix3& p_matrix);

		static FMatrix3 Transpose(const FMatrix3& p_matrix);

		static FMatrix3 Cofactor(const FMatrix3& p_matrix);


		static FMatrix3 Minor(const FMatrix3& p_matrix);

		static FMatrix3 Adjoint(const FMatrix3& p_other);

		static FMatrix3 Inverse(const FMatrix3& p_matrix);

		static FMatrix3 Translation(const FVector2& p_translation);

		static FMatrix3 Translate(const FMatrix3& p_matrix, const FVector2& p_translation);

		static FMatrix3 Rotation(float p_rotation);

		static FMatrix3 Rotate(const FMatrix3& p_matrix, float p_rotation);

		static FMatrix3 Scaling(const FVector2& p_scale);

		static FMatrix3 Scale(const FMatrix3& p_matrix, const FVector2& p_scale);

		static FVector3 GetRow(const FMatrix3& p_matrix, uint8_t p_row);

		static FVector3 GetColumn(const FMatrix3& p_matrix, uint8_t p_column);
	};
}