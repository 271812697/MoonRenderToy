#pragma once
#include <cmath>
#include <algorithm>

namespace PathTrace {
#define QUAD_LIGHT 0
#define SPHERE_LIGHT 1
#define DISTANT_LIGHT 2
	inline float PI = 3.14159265358979323;
	inline float INV_PI = 0.31830988618379067;
	inline float TWO_PI = 6.28318530717958648;
	inline float INV_TWO_PI = 0.15915494309189533;
	inline float INV_4_PI = 0.07957747154594766;
	inline float EPS = 0.0003;
	inline float INF = 1000000.0;
	struct Math
	{
	public:
		static inline float Degrees(float radians) { return radians * (180.f / PI); };
		static inline float Radians(float degrees) { return degrees * (PI / 180.f); };
		static inline float Clamp(float x, float lower, float upper) { return std::min(upper, std::max(x, lower)); };
	};
	struct Vec3;

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
		Vec2 operator*(float b) const;
		Vec2 operator+(const Vec2& b) const;
		Vec2 operator-(const Vec2& b) const;
		float x, y;
	};
	struct Vec4
	{
	public:
		Vec4();
		Vec4(float x, float y, float z, float w);
		Vec4(const Vec3& v, float w);
		Vec3 xyz();

		float operator[](int i) const;

		float x, y, z, w;
	};
	struct ivec3 {
		int x, y, z;
		ivec3(int v) :x(v), y(v), z(v) {}
	};
	struct Vec3
	{
	public:
		Vec3();
		Vec3(float v);
		Vec3(float x, float y, float z);
		Vec3(const Vec4& b);

		Vec3 operator*(const Vec3& b) const;
		Vec3& operator*=(float c);
		Vec3& operator +=(const Vec3& b);

		Vec3 operator+(const Vec3& b) const;
		Vec3 operator-(const Vec3& b) const;
		Vec3 operator-()const;
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
	Vec3 operator*(float b, const Vec3& v);
	struct Ray
	{
		Vec3 origin;
		Vec3 direction;
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
		Mat4 Transpose();
		Vec3 MulPoint(const Vec3& v);
		Vec3 MulDir(const Vec3& v);

		float data[4][4];
	};
	float Luminance(float r, float g, float b);
	
	float clamp(float x, float min, float max);
	//
	//pos the start of rectangle , plane that includes the the rectangle ,u v the two axis of rectangle and the lenght is inverse
	float RectIntersect(const Vec3& pos, const Vec3& u, const Vec3& v, const Vec4& plane, const Ray& r);
	float SphereIntersect(float rad, const Vec3& pos, const Ray& r);
	float AABBIntersect(const Vec3& minCorner, const Vec3& maxCorner, const Vec3& p, const Vec3& d);
	float AABBIntersect(const Vec3& minCorner, const Vec3& maxCorner, const Ray& r);
}