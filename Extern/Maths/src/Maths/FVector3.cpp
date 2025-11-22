#include <utility>
#include <stdexcept>
#include <cmath>

#include "Maths/FVector3.h"

const Maths::FVector3 Maths::FVector3::One(1.0f, 1.0f, 1.0f);
const Maths::FVector3 Maths::FVector3::Zero(0.0f, 0.0f, 0.0f);
const Maths::FVector3 Maths::FVector3::Forward(0.0f, 0.0f, 1.0f);
const Maths::FVector3 Maths::FVector3::Right(1.0f, 0.0f, 0.0f);
const Maths::FVector3 Maths::FVector3::Up(0.0f, 1.0f, 0.0f);
static float clamp(float t, float a, float b) {
	if (t < a)return a;
	if (t > b)return b;
	return t;
}
Maths::FVector3::FVector3(float p_x, float p_y, float p_z) : x(p_x), y(p_y), z(p_z)
{
}

Maths::FVector3::FVector3(const FVector3& p_toCopy) : x(p_toCopy.x), y(p_toCopy.y), z(p_toCopy.z)
{
}

Maths::FVector3 Maths::FVector3::Normalize()
{
	return Maths::FVector3::Normalize(*this);
}

Maths::FVector3 Maths::FVector3::operator-() const
{
	return operator*(-1.0f);
}

Maths::FVector3 Maths::FVector3::operator=(const FVector3& p_other)
{
	this->x = p_other.x;
	this->y = p_other.y;
	this->z = p_other.z;

	return *this;
}
float Maths::FVector3::operator[](int i) const
{
	if (i == 0)
		return x;
	else if (i == 1)
		return y;
	else
		return z;
}

float& Maths::FVector3::operator[](int i)
{
	if (i == 0)
		return x;
	else if (i == 1)
		return y;
	else
		return z;
}
Maths::FVector3 Maths::FVector3::operator+(const FVector3& p_other) const
{
	return Add(*this, p_other);
}

Maths::FVector3& Maths::FVector3::operator+=(const FVector3& p_other)
{
	*this = Add(*this, p_other);
	return *this;
}

Maths::FVector3 Maths::FVector3::operator-(const FVector3& p_other) const
{
	return Substract(*this, p_other);
}

Maths::FVector3& Maths::FVector3::operator-=(const FVector3& p_other)
{
	*this = Substract(*this, p_other);
	return *this;
}

Maths::FVector3 Maths::FVector3::operator*(const Maths::FVector3& p_other) const
{
	return Multiply(*this, p_other);
}

Maths::FVector3 Maths::FVector3::operator*(float p_scalar) const
{
	return Multiply(*this, p_scalar);
}

Maths::FVector3& Maths::FVector3::operator*=(float p_scalar)
{
	*this = Multiply(*this, p_scalar);
	return *this;
}

Maths::FVector3& Maths::FVector3::operator*=(const Maths::FVector3& p_other)
{
	*this = Multiply(*this, p_other);
	return *this;
}

Maths::FVector3 Maths::FVector3::operator/(float p_scalar) const
{
	return Divide(*this, p_scalar);
}

Maths::FVector3& Maths::FVector3::operator/=(float p_scalar)
{
	*this = Divide(*this, p_scalar);
	return *this;
}

bool Maths::FVector3::operator==(const FVector3 & p_other)
{
	return
		this->x == p_other.x &&
		this->y == p_other.y &&
		this->z == p_other.z;
}

bool Maths::FVector3::operator!=(const FVector3 & p_other)
{
	return !operator==(p_other);
}

float Maths::FVector3::Length()
{
	return Length(*this);
}

float Maths::FVector3::Dot(const FVector3& p_left)
{
	return Dot(p_left,*this);
}
Maths::FVector3 Maths::FVector3::Cross(const FVector3& p_right) {
	return Cross(*this,p_right);
}
float Maths::FVector3::Max()
{
	return std::max(x, std::max(y, z));
}
Maths::FVector3 Maths::FVector3::Add(const FVector3& p_left, const FVector3& p_right)
{
	return FVector3
	(
		p_left.x + p_right.x,
		p_left.y + p_right.y,
		p_left.z + p_right.z
	);
}

Maths::FVector3 Maths::FVector3::Substract(const FVector3& p_left, const FVector3& p_right)
{
	return FVector3
	(
		p_left.x - p_right.x,
		p_left.y - p_right.y,
		p_left.z - p_right.z
	);
}

Maths::FVector3 Maths::FVector3::Multiply(const FVector3& p_target, float p_scalar)
{
	return FVector3
	(
		p_target.x * p_scalar,
		p_target.y * p_scalar,
		p_target.z * p_scalar
	);
}

Maths::FVector3 Maths::FVector3::Multiply(const FVector3& p_left, const FVector3& p_right)
{
	return FVector3
	(
		p_left.x * p_right.x,
		p_left.y * p_right.y,
		p_left.z * p_right.z
	);
}

Maths::FVector3 Maths::FVector3::Divide(const FVector3& p_left, float p_scalar)
{
	FVector3 result(p_left);

	if (p_scalar == 0)
		throw std::logic_error("Division by 0");

	result.x /= p_scalar;
	result.y /= p_scalar;
	result.z /= p_scalar;

	return result;
}

float Maths::FVector3::Length(const FVector3& p_target)
{
	return std::sqrt(p_target.x * p_target.x + p_target.y * p_target.y + p_target.z * p_target.z);
}

float Maths::FVector3::Dot(const FVector3& p_left, const FVector3& p_right)
{
	return p_left.x * p_right.x + p_left.y * p_right.y + p_left.z * p_right.z;
}

float Maths::FVector3::Distance(const FVector3 & p_left, const FVector3 & p_right)
{
	return std::sqrt
	(
		(p_left.x - p_right.x) * (p_left.x - p_right.x) +
		(p_left.y - p_right.y) * (p_left.y - p_right.y) +
		(p_left.z - p_right.z) * (p_left.z - p_right.z)
	);
}

Maths::FVector3 Maths::FVector3::Cross(const FVector3 & p_left, const FVector3 & p_right)
{
	return FVector3
	(
		p_left.y * p_right.z - p_left.z * p_right.y,
		p_left.z * p_right.x - p_left.x * p_right.z,
		p_left.x * p_right.y - p_left.y * p_right.x
	);
}
Maths::FVector3 Maths::FVector3::Min(const FVector3& p_left, const FVector3& p_right)
{
	return FVector3
	(
		std::min(p_left.x,p_right.x),
		std::min(p_left.y, p_right.y),
		std::min(p_left.z, p_right.z)
	);
}
Maths::FVector3 Maths::FVector3::Max(const FVector3& p_left, const FVector3& p_right)
{
	return FVector3
	(
		std::max(p_left.x, p_right.x),
		std::max(p_left.y, p_right.y),
		std::max(p_left.z, p_right.z)
	);
}

Maths::FVector3 Maths::FVector3::Normalize(const FVector3 & p_target)
{
	float length = Length(p_target);

	if (length > 0.0f)
	{
		float targetLength = 1.0f / length;

		return FVector3
		(
			p_target.x * targetLength,
			p_target.y * targetLength,
			p_target.z * targetLength
		);
	}
	else
	{
		return FVector3::Zero;
	}
}

Maths::FVector3 Maths::FVector3::Lerp(const FVector3& p_start, const FVector3& p_end, float p_alpha)
{
	return (p_start + (p_end - p_start) * p_alpha);
}
Maths::FVector3 Maths::FVector3::Clamp(const FVector3& a, const FVector3& min, const FVector3& max) {
	return Maths::FVector3(clamp(a.x, min.x, max.x), clamp(a.y, min.y, max.y), clamp(a.z, min.z, max.z));
}
float Maths::FVector3::AngleBetween(const FVector3& p_from, const FVector3& p_to)
{
	float lengthProduct = Length(p_from) * Length(p_to);

	if (lengthProduct > 0.0f)
	{
		float fractionResult = Dot(p_from, p_to) / lengthProduct;

		if (fractionResult >= -1.0f && fractionResult <= 1.0f)
			return acosf(fractionResult);
	}

	return 0.0f;
}


