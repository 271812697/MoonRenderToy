
#pragma once


namespace Maths
{

	struct FVector3
	{
		static const FVector3 One;
		static const FVector3 Zero;
		static const FVector3 Forward;
		static const FVector3 Right;
		static const FVector3 Up;

		float x;
		float y;
		float z;


		FVector3(float p_x = 0.0f, float p_y = 0.0f, float p_z = 0.0f);

		FVector3(const FVector3& p_toCopy);


		FVector3(FVector3&& p_toMove) noexcept = default;

		FVector3 operator-() const;

		FVector3 operator=(const FVector3& p_other);

		FVector3 operator+(const FVector3& p_other) const;

		FVector3& operator+=(const FVector3& p_other);

		FVector3 operator-(const FVector3& p_other) const;


		FVector3& operator-=(const FVector3& p_other);

		FVector3 operator*(float p_scalar) const;

		FVector3& operator*=(float p_scalar);

		FVector3 operator/(float p_scalar) const;


		FVector3& operator/=(float p_scalar);


		bool operator==(const FVector3& p_other);

		bool operator!=(const FVector3& p_other);


		static FVector3 Add(const FVector3& p_left, const FVector3& p_right);

		static FVector3 Substract(const FVector3& p_left, const FVector3& p_right);


		static FVector3 Multiply(const FVector3& p_target, float p_scalar);

		static FVector3 Divide(const FVector3& p_left, float p_scalar);

		static float Length(const FVector3& p_target);

		static float Dot(const FVector3& p_left, const FVector3& p_right);

		static float Distance(const FVector3& p_left, const FVector3& p_right);

		static FVector3 Cross(const FVector3& p_left, const FVector3& p_right);

		static FVector3 Normalize(const FVector3& p_target);

		static FVector3 Lerp(const FVector3& p_start, const FVector3& p_end, float p_alpha);

		static float AngleBetween(const FVector3& p_from, const FVector3& p_to);
	};
}