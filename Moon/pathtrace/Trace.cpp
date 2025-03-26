#include "Trace.h"
#include "RendererOptions.h"
#include "Scene.h"

namespace PathTrace
{



	static RenderOptions renderOpt;
	static Scene* scene = nullptr;
	bool ClosestHit(Ray r, State& state, LightSampleRec& lightSample) {
		float t = INF;
		float d;
		if ((renderOpt.hideEmitters && state.depth > 0) || !renderOpt.hideEmitters) {
			for (int i = 0; i < scene->lights.size(); i++) {
				Vec3 position = scene->lights[i].position;
				Vec3 emission = scene->lights[i].emission;
				Vec3 u = scene->lights[i].u;
				Vec3 v = scene->lights[i].v;

				float radius = scene->lights[i].radius;
				float area = scene->lights[i].area;
				float type = scene->lights[i].type;
				if (type == QUAD_LIGHT) {

					Vec3 normal = Vec3::Normalize(Vec3::Cross(u, v));
					if (Vec3::Dot(normal, r.direction) > 0.0) {
						continue;
					}
					Vec4 plane = Vec4(normal, Vec3::Dot(normal, position));
					u *= (1.0f / Vec3::Dot(u, u));
					v *= (1.0f / Vec3::Dot(v, v));
					d = RectIntersect(position, u, v, plane, r);
					if (d < 0.)
						d = INF;
					if (d < t)
					{
						t = d;
						float cosTheta = Vec3::Dot(-r.direction, normal);
						lightSample.pdf = (t * t) / (area * cosTheta);
						lightSample.emission = emission;
						state.isEmitter = true;
					}
				}
				if (type == SPHERE_LIGHT)
				{
					d = SphereIntersect(radius, position, r);
					if (d < 0.)
						d = INF;
					if (d < t)
					{
						t = d;
						Vec3 hitPt = r.origin + t * r.direction;
						float cosTheta = Vec3::Dot(-r.direction, Vec3::Normalize(hitPt - position));
						// TODO: Fix this. Currently assumes the light will be hit only from the outside
						lightSample.pdf = (t * t) / (area * cosTheta * 0.5);
						lightSample.emission = emission;
						state.isEmitter = true;
					}
				}
			}
		}

		return false;
	}
	Vec4 Trace(const Ray& r)
	{
		Vec3 radiance = Vec3(0.0);
		Vec3 throughput = Vec3(1.0);

		State state;
		LightSampleRec lightSample;
		ScatterSampleRec scatterSample;
		// FIXME: alpha from material opacity/medium density
		float alpha = 1.0;
		// For medium tracking
		bool inMedium = false;
		bool mediumSampled = false;
		bool surfaceScatter = false;
		for (state.depth = 0;; state.depth++) {

		}

		return Vec4();
	}
}