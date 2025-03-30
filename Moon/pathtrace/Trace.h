#pragma once
#include "Material.h"
#include "MathUtil.h"
namespace PathTrace
{
	struct RenderOptions;

	struct Medium
	{
		int type;
		float density;
		Vec3 color;
		float anisotropy;
	};

	enum LightType
	{
		RectLight,
		SphereLight,
		DistantLight
	};

	struct Light
	{
		Vec3 position;
		Vec3 emission;
		Vec3 u;
		Vec3 v;
		float radius;
		float area;
		float type;
	};

	struct State
	{
		int depth;
		float eta;
		float hitDist;

		Vec3 fhp;
		Vec3 normal;
		Vec3 ffnormal;
		Vec3 tangent;
		Vec3 bitangent;

		bool isEmitter;

		Vec2 texCoord;
		int matID;
		Material mat;
		Medium medium;
	};

	struct ScatterSampleRec
	{
		Vec3 L;
		Vec3 f;
		float pdf;
	};

	struct LightSampleRec
	{
		Vec3 normal;
		Vec3 emission;
		Vec3 direction;
		float dist;
		float pdf;
	};
	Vec4 Trace(Ray& r);
	void TraceScreen(int width, int height);

}
