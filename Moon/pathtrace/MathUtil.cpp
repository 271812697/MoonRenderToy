#pragma once
#include "MathUtil.h"
#define PI 3.14159265358979323846f
namespace PathTrace {

	Vec4::Vec4()
	{
		x = y = z = w = 0;
	};

	Vec4::Vec4(float _x, float _y, float _z, float _w) :
		x(_x), y(_y), z(_z), w(_w)
	{
	}

	Vec4::Vec4(const Vec3& v, float _w) :
		x(v.x), y(v.y), z(v.z), w(_w)
	{
	}

	Vec3 Vec4::xyz()
	{
		return Vec3(x, y, z);
	}

	Vec4 Vec4::operator*(float b) const
	{
		return Vec4(x * b, y * b, z * b, w * b);
	}
	Vec4 Vec4::operator+(const Vec4& b) const
	{
		return Vec4(x + b.x, y + b.y, z + b.z, w + b.w);
	}

	float Vec4::operator[](int i) const
	{
		if (i == 0)
			return x;
		else if (i == 1)
			return y;
		else if (i == 2)
			return z;
		else
			return w;
	};


	Vec3::Vec3()
	{
		x = y = z = 0;
	}
	Vec3::Vec3(float v)
	{
		x = y = z = v;
	}
	;

	Vec3::Vec3(float _x, float _y, float _z) :
		x(_x), y(_y), z(_z)
	{
	}

	Vec3::Vec3(const Vec4& b)
	{
		x = b.x;
		y = b.y;
		z = b.z;
	};

	Vec3 Vec3::operator*(const Vec3& b) const
	{
		return Vec3(x * b.x, y * b.y, z * b.z);
	}
	Vec3& Vec3::operator*=(float c)
	{
		x *= c;
		y *= c;
		z *= c;
		return *this;

	}
	Vec3& Vec3::operator/=(float c)
	{
		x /= c;
		y /= c;
		z /= c;
		return *this;

	}
	Vec3& Vec3::operator*=(const Vec3 c)
	{
		x *= c.x;
		y *= c.y;
		z *= c.z;
		return *this;
	}
	Vec3& Vec3::operator+=(const Vec3& b) {
		x += b.x;
		y += b.y;
		z += b.z;
		return *this;
	}

	Vec3 Vec3::operator+(const Vec3& b) const
	{
		return Vec3(x + b.x, y + b.y, z + b.z);
	};

	Vec3 Vec3::operator-(const Vec3& b) const
	{
		return Vec3(x - b.x, y - b.y, z - b.z);
	}
	Vec3 Vec3::operator-(float c) const
	{
		return Vec3(x - c, y - c, z - c);
	}
	;
	Vec3 Vec3::operator-()const {
		return Vec3(-x, -y, -z);
	}

	Vec3 Vec3::operator*(float b) const
	{
		return Vec3(x * b, y * b, z * b);
	};
	Vec3 Vec3::operator/(float b) const
	{
		return Vec3(x / b, y / b, z / b);
	};

	float Vec3::operator[](int i) const
	{
		if (i == 0)
			return x;
		else if (i == 1)
			return y;
		else
			return z;
	};
	Vec3 Vec3::operator/(const Vec3& b) const {
		return Vec3(x / b.x, y / b.y, z / b.z);
	}

	float& Vec3::operator[](int i)
	{
		if (i == 0)
			return x;
		else if (i == 1)
			return y;
		else
			return z;
	};

	Vec3 Vec3::Min(const Vec3& a, const Vec3& b)
	{
		Vec3 out;
		out.x = std::min(a.x, b.x);
		out.y = std::min(a.y, b.y);
		out.z = std::min(a.z, b.z);
		return out;
	};

	Vec3 Vec3::Max(const Vec3& a, const Vec3& b)
	{
		Vec3 out;
		out.x = std::max(a.x, b.x);
		out.y = std::max(a.y, b.y);
		out.z = std::max(a.z, b.z);
		return out;
	};

	Vec3 Vec3::Cross(const Vec3& a, const Vec3& b)
	{
		return Vec3(
			a.y * b.z - a.z * b.y,
			a.z * b.x - a.x * b.z,
			a.x * b.y - a.y * b.x
		);
	};

	Vec3 Vec3::Pow(const Vec3& a, float exp)
	{
		return Vec3(
			powf(a.x, exp),
			powf(a.y, exp),
			powf(a.z, exp)
		);
	};

	float Vec3::Length(const Vec3& a)
	{
		return sqrtf(a.x * a.x + a.y * a.y + a.z * a.z);
	};

	float Vec3::Dot(const Vec3& a, const Vec3& b)
	{
		return (a.x * b.x + a.y * b.y + a.z * b.z);
	};

	float Vec3::Distance(const Vec3& a, const Vec3& b)
	{
		Vec3 t = a;
		return Length(t - b);
	}

	Vec3 Vec3::Clamp(const Vec3& a, const Vec3& min, const Vec3& max)
	{
		return Vec3(
			Math::Clamp(a.x, min.x, max.x),
			Math::Clamp(a.y, min.y, max.y),
			Math::Clamp(a.z, min.z, max.z)
		);
	}

	Vec3 Vec3::Normalize(const Vec3& a)
	{
		float l = Length(a);
		return Vec3(a.x / l, a.y / l, a.z / l);
	}
	Mat4::Mat4()
	{
		data[0][0] = 1; data[0][1] = 0;  data[0][2] = 0;  data[0][3] = 0;
		data[1][0] = 0; data[1][1] = 1;  data[1][2] = 0;  data[1][3] = 0;
		data[2][0] = 0; data[2][1] = 0;  data[2][2] = 1;  data[2][3] = 0;
		data[3][0] = 0; data[3][1] = 0;  data[3][2] = 0;  data[3][3] = 1;
	}

	Mat4 Mat4::Translate(const Vec3& a)
	{
		Mat4 out;
		out[3][0] = a.x;
		out[3][1] = a.y;
		out[3][2] = a.z;
		return out;
	}

	Mat4 Mat4::Scale(const Vec3& a)
	{
		Mat4 out;
		out[0][0] = a.x;
		out[1][1] = a.y;
		out[2][2] = a.z;
		return out;
	}

	Mat4 Mat4::operator*(const Mat4& b) const
	{
		Mat4 out;

		out[0][0] = data[0][0] * b.data[0][0] + data[0][1] * b.data[1][0] + data[0][2] * b.data[2][0] + data[0][3] * b.data[3][0];
		out[0][1] = data[0][0] * b.data[0][1] + data[0][1] * b.data[1][1] + data[0][2] * b.data[2][1] + data[0][3] * b.data[3][1];
		out[0][2] = data[0][0] * b.data[0][2] + data[0][1] * b.data[1][2] + data[0][2] * b.data[2][2] + data[0][3] * b.data[3][2];
		out[0][3] = data[0][0] * b.data[0][3] + data[0][1] * b.data[1][3] + data[0][2] * b.data[2][3] + data[0][3] * b.data[3][3];

		out[1][0] = data[1][0] * b.data[0][0] + data[1][1] * b.data[1][0] + data[1][2] * b.data[2][0] + data[1][3] * b.data[3][0];
		out[1][1] = data[1][0] * b.data[0][1] + data[1][1] * b.data[1][1] + data[1][2] * b.data[2][1] + data[1][3] * b.data[3][1];
		out[1][2] = data[1][0] * b.data[0][2] + data[1][1] * b.data[1][2] + data[1][2] * b.data[2][2] + data[1][3] * b.data[3][2];
		out[1][3] = data[1][0] * b.data[0][3] + data[1][1] * b.data[1][3] + data[1][2] * b.data[2][3] + data[1][3] * b.data[3][3];

		out[2][0] = data[2][0] * b.data[0][0] + data[2][1] * b.data[1][0] + data[2][2] * b.data[2][0] + data[2][3] * b.data[3][0];
		out[2][1] = data[2][0] * b.data[0][1] + data[2][1] * b.data[1][1] + data[2][2] * b.data[2][1] + data[2][3] * b.data[3][1];
		out[2][2] = data[2][0] * b.data[0][2] + data[2][1] * b.data[1][2] + data[2][2] * b.data[2][2] + data[2][3] * b.data[3][2];
		out[2][3] = data[2][0] * b.data[0][3] + data[2][1] * b.data[1][3] + data[2][2] * b.data[2][3] + data[2][3] * b.data[3][3];

		out[3][0] = data[3][0] * b.data[0][0] + data[3][1] * b.data[1][0] + data[3][2] * b.data[2][0] + data[3][3] * b.data[3][0];
		out[3][1] = data[3][0] * b.data[0][1] + data[3][1] * b.data[1][1] + data[3][2] * b.data[2][1] + data[3][3] * b.data[3][1];
		out[3][2] = data[3][0] * b.data[0][2] + data[3][1] * b.data[1][2] + data[3][2] * b.data[2][2] + data[3][3] * b.data[3][2];
		out[3][3] = data[3][0] * b.data[0][3] + data[3][1] * b.data[1][3] + data[3][2] * b.data[2][3] + data[3][3] * b.data[3][3];

		return out;
	}

	Mat4 Mat4::QuatToMatrix(float x, float y, float z, float w)
	{
		Mat4 out;
		const float x2 = x + x;
		const float y2 = y + y;
		const float z2 = z + z;

		const float xx = x * x2;
		const float xy = x * y2;
		const float xz = x * z2;

		const float yy = y * y2;
		const float yz = y * z2;
		const float zz = z * z2;

		const float wx = w * x2;
		const float wy = w * y2;
		const float wz = w * z2;

		out.data[0][0] = 1.0f - (yy + zz);
		out.data[0][1] = xy + wz;
		out.data[0][2] = xz - wy;
		out.data[0][3] = 0.0f;

		out.data[1][0] = xy - wz;
		out.data[1][1] = 1.0f - (xx + zz);
		out.data[1][2] = yz + wx;
		out.data[1][3] = 0.0f;

		out.data[2][0] = xz + wy;
		out.data[2][1] = yz - wx;
		out.data[2][2] = 1.0f - (xx + yy);
		out.data[2][3] = 0.0f;

		out.data[3][0] = 0;
		out.data[3][1] = 0;
		out.data[3][2] = 0;
		out.data[3][3] = 1.0f;

		return out;
	}

	Mat4 Mat4::Inverse()
	{
		float m00 = data[0][0], m01 = data[1][0], m02 = data[2][0], m03 = data[3][0];
		float m10 = data[0][1], m11 = data[1][1], m12 = data[2][1], m13 = data[3][1];
		float m20 = data[0][2], m21 = data[1][2], m22 = data[2][2], m23 = data[3][2];
		float m30 = data[0][3], m31 = data[1][3], m32 = data[2][3], m33 = data[3][3];

		float v0 = m20 * m31 - m21 * m30;
		float v1 = m20 * m32 - m22 * m30;
		float v2 = m20 * m33 - m23 * m30;
		float v3 = m21 * m32 - m22 * m31;
		float v4 = m21 * m33 - m23 * m31;
		float v5 = m22 * m33 - m23 * m32;

		float t00 = +(v5 * m11 - v4 * m12 + v3 * m13);
		float t10 = -(v5 * m10 - v2 * m12 + v1 * m13);
		float t20 = +(v4 * m10 - v2 * m11 + v0 * m13);
		float t30 = -(v3 * m10 - v1 * m11 + v0 * m12);

		float invDet = 1 / (t00 * m00 + t10 * m01 + t20 * m02 + t30 * m03);

		float d00 = t00 * invDet;
		float d10 = t10 * invDet;
		float d20 = t20 * invDet;
		float d30 = t30 * invDet;

		float d01 = -(v5 * m01 - v4 * m02 + v3 * m03) * invDet;
		float d11 = +(v5 * m00 - v2 * m02 + v1 * m03) * invDet;
		float d21 = -(v4 * m00 - v2 * m01 + v0 * m03) * invDet;
		float d31 = +(v3 * m00 - v1 * m01 + v0 * m02) * invDet;

		v0 = m10 * m31 - m11 * m30;
		v1 = m10 * m32 - m12 * m30;
		v2 = m10 * m33 - m13 * m30;
		v3 = m11 * m32 - m12 * m31;
		v4 = m11 * m33 - m13 * m31;
		v5 = m12 * m33 - m13 * m32;

		float d02 = +(v5 * m01 - v4 * m02 + v3 * m03) * invDet;
		float d12 = -(v5 * m00 - v2 * m02 + v1 * m03) * invDet;
		float d22 = +(v4 * m00 - v2 * m01 + v0 * m03) * invDet;
		float d32 = -(v3 * m00 - v1 * m01 + v0 * m02) * invDet;

		v0 = m21 * m10 - m20 * m11;
		v1 = m22 * m10 - m20 * m12;
		v2 = m23 * m10 - m20 * m13;
		v3 = m22 * m11 - m21 * m12;
		v4 = m23 * m11 - m21 * m13;
		v5 = m23 * m12 - m22 * m13;

		float d03 = -(v5 * m01 - v4 * m02 + v3 * m03) * invDet;
		float d13 = +(v5 * m00 - v2 * m02 + v1 * m03) * invDet;
		float d23 = -(v4 * m00 - v2 * m01 + v0 * m03) * invDet;
		float d33 = +(v3 * m00 - v1 * m01 + v0 * m02) * invDet;


		Mat4 ans;
		ans.data[0][0] = d00, ans.data[0][1] = d10, ans.data[0][2] = d20, ans.data[0][3] = d30;
		ans.data[1][0] = d01, ans.data[1][1] = d11, ans.data[1][2] = d21, ans.data[1][3] = d31;
		ans.data[2][0] = d02, ans.data[2][1] = d12, ans.data[2][2] = d22, ans.data[2][3] = d32;
		ans.data[3][0] = d03, ans.data[3][1] = d13, ans.data[3][2] = d23, ans.data[3][3] = d33;
		return ans;
	}

	Mat4 Mat4::Transpose()
	{
		Mat4 res;
		for (int i = 0; i < 4; i++) {
			for (int j = 0; j < 4; j++) {
				res.data[i][j] = data[j][i];
			}
		}
		return res;
	}

	Vec3 Mat4::MulPoint(const Vec3& v)
	{
		return Vec3(
			data[0][0] * v.x + data[1][0] * v.y + data[2][0] * v.z + data[3][0],
			data[0][1] * v.x + data[1][1] * v.y + data[2][1] * v.z + data[3][1],
			data[0][2] * v.x + data[1][2] * v.y + data[2][2] * v.z + data[3][2]
		);
	}

	Vec3 Mat4::MulDir(const Vec3& v)
	{
		return Vec3(
			data[0][0] * v.x + data[1][0] * v.y + data[2][0] * v.z,
			data[0][1] * v.x + data[1][1] * v.y + data[2][1] * v.z,
			data[0][2] * v.x + data[1][2] * v.y + data[2][2] * v.z
		);
	}

	Vec4 operator*(float b, const Vec4& v)
	{
		return Vec4(b * v.x, b * v.y, b * v.z, b * v.w);
	}

	Vec3 operator*(float b, const Vec3& v)
	{
		return Vec3(b * v.x, b * v.y, b * v.z);
	}
	Vec3 pow(const Vec3& a, const Vec3& b)
	{
		return Vec3(std::pow(a.x, b.x), std::pow(a.y, b.y), std::pow(a.z, b.z));
	}
	Vec3 RoateDir(const Vec3& d, const Vec3& axis, float rad)
	{
		Vec3 n = Vec3::Dot(d, axis) * axis;
		Vec3 x = d - n;
		float len = Vec3::Length(x);
		if (len < FLT_EPSILON) {
			return d;
		}
		Vec3 nx = Vec3::Normalize(x);
		Vec3 ny = Vec3::Cross(axis, nx);
		return n + len * (cos(rad) * nx + sin(rad) * ny);
	}
	Vec3 RoatePoint(const Vec3& p, const Vec3& axis, const Vec3& pivot, float rad)
	{
		Vec3 d = p - pivot;
		Vec3 dir = RoateDir(d, axis, rad);
		return pivot + dir;
	}
	uvec4 seed;
	void InitRNG(Vec2 p, int frame)
	{
		seed.x = p.x;
		seed.y = p.y;
		seed.z = frame;
		seed.w = p.x + p.y;
		//seed = { ,p.y,frame,};// uvec4(p, uint(frame), uint(p.x) + uint(p.y));
	}
	void pcg4d()
	{
		seed.x = seed.x * 1664525 + 1013904223;
		seed.y = seed.y * 1664525 + 1013904223;
		seed.z = seed.z * 1664525u + 1013904223u;
		seed.w = seed.w * 1664525u + 1013904223u;
		seed.x += seed.y * seed.w; seed.y += seed.z * seed.x; seed.z += seed.x * seed.y; seed.w += seed.y * seed.z;
		seed.x = seed.x ^ (seed.x >> 16);
		seed.y = seed.y ^ (seed.y >> 16);
		seed.z = seed.z ^ (seed.z >> 16);
		seed.w = seed.w ^ (seed.w >> 16);
		seed.x += seed.y * seed.w; seed.y += seed.z * seed.x; seed.z += seed.x * seed.y; seed.w += seed.y * seed.z;
	}
	float uniform_float()
	{
		pcg4d();
		return  float(seed.x) / float(0xffffffffu);
	}
	float PhaseHG(float cosTheta, float g)
	{
		float denom = 1 + g * g + 2 * g * cosTheta;
		return INV_4_PI * (1 - g * g) / (denom * sqrt(denom));
	}

	float Luminance(float r, float g, float b)
	{
		return 0.212671f * r + 0.715160f * g + 0.072169f * b;
	}
	float Luminance(const Vec3& a) {
		return Luminance(a.x, a.y, a.z);
	}
	float PowerHeuristic(float a, float b)
	{
		float t = a * a;
		return t / (b * b + t);
	}

	float clamp(float x, float min, float max) {
		if (x < min) return min;
		if (x > max) return max;
		return x;
	}
	Vec3 clamp(const Vec3& v, float min, float max)
	{
		return Vec3(clamp(v.x, min, max), clamp(v.y, min, max), clamp(v.z, min, max));
	}
	Vec3 exp(const Vec3& epo)
	{
		return Vec3(std::exp(epo.x), std::exp(epo.y), std::exp(epo.z));
	}
	Vec3 mix(const Vec3& a, const Vec3& b, float c) {
		return a * (1 - c) + c * b;
	}
	float mix(float a, float b, float alpha) {
		return a * (1 - alpha) + alpha * b;
	}

	Vec3 reflect(const Vec3& v, const Vec3& n)
	{
		return v - 2 * Vec3::Dot(v, n) * n;
	}

	Vec3 refract(const Vec3& uv, const Vec3& n, float etai_over_etat)
	{
		auto cos_theta = fmin(Vec3::Dot(-uv, n), 1.0);
		Vec3 r_out_perp = etai_over_etat * (uv + cos_theta * n);
		Vec3 r_out_parallel = -sqrt(fabs(1.0 - Vec3::Dot(r_out_perp, r_out_perp))) * n;
		return r_out_perp + r_out_parallel;
	}

	float RectIntersect(const Vec3& pos, const Vec3& u, const Vec3& v, const Vec4& plane, const Ray& r)
	{
		Vec3 n = Vec3(plane);
		float dt = Vec3::Dot(r.direction, n);
		float t = (plane.w - Vec3::Dot(n, r.origin)) / dt;

		if (t > EPS)
		{
			Vec3 p = r.origin + r.direction * t;
			Vec3 vi = p - pos;
			float a1 = Vec3::Dot(u, vi);
			if (a1 >= 0.0 && a1 <= 1.0)
			{
				float a2 = Vec3::Dot(v, vi);
				if (a2 >= 0.0 && a2 <= 1.0)
					return t;
			}
		}

		return INF;
	}

	float SphereIntersect(float rad, const Vec3& pos, const Ray& r)
	{
		Vec3 op = pos - r.origin;
		float eps = 0.001;
		float b = Vec3::Dot(op, r.direction);
		float det = b * b - Vec3::Dot(op, op) + rad * rad;
		if (det < 0.0)
			return INF;

		det = sqrt(det);
		float t1 = b - det;
		if (t1 > eps)
			return t1;

		float t2 = b + det;
		if (t2 > eps)
			return t2;

		return INF;
	}

	float AABBIntersect(const Vec3& minCorner, const Vec3& maxCorner, const Vec3& p, const Vec3& d)
	{
		Vec3 invDir = { 1.0f / d.x,1.0f / d.y, 1.0f / d.z };

		Vec3 f = (maxCorner - p) * invDir;
		Vec3 n = (minCorner - p) * invDir;

		Vec3 tmax = Vec3::Max(f, n);
		Vec3 tmin = Vec3::Min(f, n);

		float t1 = std::min(tmax.x, std::min(tmax.y, tmax.z));
		float t0 = std::max(tmin.x, std::max(tmin.y, tmin.z));

		return (t1 >= t0) ? (t0 > 0.f ? t0 : t1) : -1.0;
	}

	float AABBIntersect(const Vec3& minCorner, const Vec3& maxCorner, const Ray& r)
	{
		return AABBIntersect(minCorner, maxCorner, r.origin, r.direction);
	}

	Vec2 Vec2::operator*(float b) const
	{
		return Vec2(x * b, y * b);
	}
	Vec2 Vec2::operator+(const Vec2& b)const {
		return Vec2(x + b.x, y + b.y);
	}
	Vec2 Vec2::operator-(const Vec2& b)const {
		return Vec2(x - b.x, y - b.y);
	}

}