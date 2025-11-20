#pragma once
namespace Maths
{
	struct FVector2
	{
		static const FVector2 One;
		static const FVector2 Zero;
		float x;
		float y;
		FVector2(float p_x = 0.0f, float p_y = 0.0f);
		FVector2(const FVector2& p_toCopy);
		FVector2(FVector2&& p_toMove) noexcept = default;
		FVector2 operator-() const;
		FVector2 operator=(const FVector2& p_other);
		FVector2 operator+(const FVector2& p_other) const;
		FVector2& operator+=(const FVector2& p_other);
		FVector2 operator-(const FVector2& p_other) const;
		FVector2& operator-=(const FVector2& p_other);
		FVector2 operator*(float p_scalar) const;
		FVector2& operator*=(float p_scalar);
		FVector2 operator/(float p_scalar) const;
		FVector2& operator/=(float p_scalar);
		bool operator==(const FVector2& p_other);
		bool operator!=(const FVector2& p_other);
		static FVector2 Add(const FVector2& p_left, const FVector2& p_right);
		static FVector2 Substract(const FVector2& p_left, const FVector2& p_right);
		static FVector2 Multiply(const FVector2& p_target, float p_scalar);
		static FVector2 Divide(const FVector2& p_left, float p_scalar);
		static float Length(const FVector2& p_target);
		static float Dot(const FVector2& p_left, const FVector2& p_right);
		static FVector2 Normalize(const FVector2& p_target);
		static FVector2 Lerp(const FVector2& p_start, const FVector2& p_end, float p_alpha);
		static float AngleBetween(const FVector2& p_from, const FVector2& p_to);
	};
}