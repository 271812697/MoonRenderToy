#pragma once
#include <cmath>
#include <algorithm>
#define PI 3.14159265358979323846f
namespace PathTrace {
	struct Math
	{
	public:
		static inline float Degrees(float radians) { return radians * (180.f / PI); };
		static inline float Radians(float degrees) { return degrees * (PI / 180.f); };
		static inline float Clamp(float x, float lower, float upper) { return std::min(upper, std::max(x, lower)); };
	};
	struct iVec2
	{
	public:
		iVec2() { x = 0, y = 0; };
		iVec2(int x, int y) { this->x = x; this->y = y; }
		int x, y;
	};

	struct Vec2
	{
	public:
		Vec2() { x = 0, y = 0; };
		Vec2(float x, float y) { this->x = x; this->y = y; };

		float x, y;
	};
	struct Vec4
	{
	public:
		Vec4();
		Vec4(float x, float y, float z, float w);

		float operator[](int i) const;

		float x, y, z, w;
	};

	struct Vec3
	{
	public:
		Vec3();
		Vec3(float v);
		Vec3(float x, float y, float z);
		Vec3(const Vec4& b);

		Vec3 operator*(const Vec3& b) const;

		Vec3 operator+(const Vec3& b) const;
		Vec3 operator-(const Vec3& b) const;
		Vec3 operator*(float b) const;

		float operator[](int i) const;
		float& operator[](int i);

		static Vec3 Min(const Vec3& a, const Vec3& b);
		static Vec3 Max(const Vec3& a, const Vec3& b);
		static Vec3 Cross(const Vec3& a, const Vec3& b);
		static Vec3 Pow(const Vec3& a, float exp);
		static float Length(const Vec3& a);
		static float Distance(const Vec3& a, const Vec3& b);
		static float Dot(const Vec3& a, const Vec3& b);
		static Vec3 Clamp(const Vec3& a, const Vec3& min, const Vec3& max);
		static Vec3 Normalize(const Vec3& a);

		float x, y, z;
	};

	struct Mat4
	{

	public:
		Mat4();

		float(&operator [](int i))[4] { return data[i]; };
		Mat4 operator*(const Mat4& b) const;

		static Mat4 Translate(const Vec3& a);
		static Mat4 Scale(const Vec3& a);
		static Mat4 QuatToMatrix(float x, float y, float z, float w);
		Mat4 Inverse();
		Vec3 MulPoint(const Vec3& v);
		Vec3 MulDir(const Vec3& v);

		float data[4][4];
	};
}