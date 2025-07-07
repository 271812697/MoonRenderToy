
#pragma once


namespace Maths
{

	struct FVector4
	{
		static const FVector4 One;
		static const FVector4 Zero;

		float x;
		float y;
		float z;
		float w;

		FVector4(float p_x = 0.0f, float p_y = 0.0f, float p_z = 0.0f, float p_w = 0.0f);

		FVector4(const FVector4& p_toCopy);

		FVector4(FVector4&& p_toMove) noexcept = default;

		FVector4 operator-() const;

		FVector4 operator=(const FVector4& p_other);

		FVector4 operator+(const FVector4& p_other) const;

		FVector4& operator+=(const FVector4& p_other);

		FVector4 operator-(const FVector4& p_other) const;

		FVector4& operator-=(const FVector4& p_other);

		FVector4 operator*(float p_scalar) const;

		FVector4& operator*=(float p_scalar);

		FVector4 operator/(float p_scalar) const;

		FVector4& operator/=(float p_scalar);

		bool operator==(const FVector4& p_other);

		bool operator!=(const FVector4& p_other);

		static FVector4 Add(const FVector4& p_left, const FVector4& p_right);

		static FVector4 Substract(const FVector4& p_left, const FVector4& p_right);

		static FVector4 Multiply(const FVector4& p_target, float p_scalar);

		static FVector4 Divide(const FVector4& p_left, float p_scalar);

		static float Length(const FVector4& p_target);

		static float Dot(const FVector4& p_left, const FVector4& p_right);

		static FVector4 Normalize(const FVector4& p_target);

		static FVector4 Lerp(const FVector4& p_start, const FVector4& p_end, float p_alpha);
	};
}