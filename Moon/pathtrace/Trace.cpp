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
		// Intersect BVH and tris
		int stack[64];
		int ptr = 0;
		stack[ptr++] = -1;

		int index = scene->bvhTranslator.topLevelIndex;
		float leftHit = 0.0;
		float rightHit = 0.0;

		int currMatID = 0;
		bool BLAS = false;

		ivec3 triID = ivec3(-1);
		Mat4 transMat;
		Mat4 transform;
		Vec3 bary;
		Vec4 vert0, vert1, vert2;


		Ray rTrans;
		rTrans.origin = r.origin;
		rTrans.direction = r.direction;


		while (index != 1)
		{
			Vec3 LRLeaf = scene->bvhTranslator.nodes[index].LRLeaf;
			int leftIndex = int(LRLeaf.x);
			int rightIndex = int(LRLeaf.y);
			int leaf = int(LRLeaf.z);
			if (leaf > 0) // Leaf node of BLAS
			{
				for (int i = 0; i < rightIndex; i++) // Loop through tris
				{

					Vec4 v0 = scene->verticesUVX[scene->vertIndices[leftIndex + i].x];
					Vec4 v1 = scene->verticesUVX[scene->vertIndices[leftIndex + i].y];
					Vec4 v2 = scene->verticesUVX[scene->vertIndices[leftIndex + i].z];

					Vec3 e0 = { v1.x - v0.x, v1.y - v0.y, v1.z - v0.z };
					Vec3 e1 = { v2.x - v0.x,v2.y - v0.y,v2.z - v0.z };
					Vec3 pv = Vec3::Cross(rTrans.direction, e1);
					float det = Vec3::Dot(e0, pv);

					Vec3 tv = { rTrans.origin.x - v0.x ,rTrans.origin.y - v0.y ,rTrans.origin.z - v0.z };
					Vec3 qv = Vec3::Cross(tv, e0);

					Vec4 uvt;
					uvt.x = Vec3::Dot(tv, pv);
					uvt.y = Vec3::Dot(rTrans.direction, qv);
					uvt.z = Vec3::Dot(e1, qv);
					uvt.x = uvt.x / det;
					uvt.y = uvt.y / det;
					uvt.z = uvt.z / det;
					uvt.w = 1.0 - uvt.x - uvt.y;

					if (uvt.x >= 0 && uvt.y >= 0 && uvt.z >= 0 && uvt.w >= 0 && uvt.z < t)
					{
						t = uvt.z;
						triID.x = scene->vertIndices[leftIndex + i].x;
						triID.y = scene->vertIndices[leftIndex + i].y;
						triID.z = scene->vertIndices[leftIndex + i].z;
						bary = { uvt.w ,uvt.x,uvt.y };
						vert0 = v0, vert1 = v1, vert2 = v2;

						transform = transMat;
					}
				}
			}
			else if (leaf < 0) {
				transMat = scene->transforms[-leaf - 1];
				auto tempMat = transMat.Inverse();
				rTrans.origin = tempMat.MulPoint(r.origin);
				rTrans.direction = tempMat.MulDir(r.direction);
				stack[ptr++] = -1;
				index = leftIndex;
				BLAS = true;
				currMatID = rightIndex;
				continue;
			}
			else
			{
				leftHit = AABBIntersect(scene->bvhTranslator.nodes[leftIndex].bboxmin, scene->bvhTranslator.nodes[leftIndex].bboxmax, rTrans);
				rightHit = AABBIntersect(scene->bvhTranslator.nodes[rightIndex].bboxmin, scene->bvhTranslator.nodes[rightIndex].bboxmax, rTrans);

				if (leftHit > 0.0 && rightHit > 0.0)
				{
					int deferred = -1;
					if (leftHit > rightHit)
					{
						index = rightIndex;
						deferred = leftIndex;
					}
					else
					{
						index = leftIndex;
						deferred = rightIndex;
					}

					stack[ptr++] = deferred;
					continue;
				}
				else if (leftHit > 0.)
				{
					index = leftIndex;
					continue;
				}
				else if (rightHit > 0.)
				{
					index = rightIndex;
					continue;
				}
			}
			index = stack[--ptr];

			// If we've traversed the entire BLAS then switch to back to TLAS and resume where we left off
			if (BLAS && index == -1)
			{
				BLAS = false;

				index = stack[--ptr];

				rTrans.origin = r.origin;
				rTrans.direction = r.direction;
			}
		}

		// No intersections
		if (t == INF)
			return false;
		state.hitDist = t;
		state.fhp = r.origin + r.direction * t;

		// Ray hit a triangle and not a light source
		if (triID.x != -1)
		{
			state.isEmitter = false;

			// Normals

			Vec4 n0 = scene->normalsUVY[triID.x];
			Vec4 n1 = scene->normalsUVY[triID.y];
			Vec4 n2 = scene->normalsUVY[triID.z];

			// Get texcoords from w coord of vertices and normals
			Vec2 t0 = Vec2(vert0.w, n0.w);
			Vec2 t1 = Vec2(vert1.w, n1.w);
			Vec2 t2 = Vec2(vert2.w, n2.w);

			// Interpolate texture coords and normals using barycentric coords
			state.texCoord = t0 * bary.x + t1 * bary.y + t2 * bary.z;
			Vec3 normal = Vec3::Normalize(n0.xyz() * bary.x + n1.xyz() * bary.y + n2.xyz() * bary.z);

			state.normal = transform.Inverse().Transpose().MulDir(normal);
			state.ffnormal = Vec3::Dot(state.normal, r.direction) <= 0.0 ? state.normal : -state.normal;

			// Calculate tangent and bitangent
			Vec3 deltaPos1 = vert1.xyz() - vert0.xyz();
			Vec3 deltaPos2 = vert2.xyz() - vert0.xyz();

			Vec2 deltaUV1 = t1 - t0;
			Vec2 deltaUV2 = t2 - t0;

			float invdet = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV1.y * deltaUV2.x);

			state.tangent = (deltaPos1 * deltaUV2.y - deltaPos2 * deltaUV1.y) * invdet;
			state.bitangent = (deltaPos2 * deltaUV1.x - deltaPos1 * deltaUV2.x) * invdet;

			state.tangent = Vec3::Normalize(transform.MulDir(state.tangent));
			state.bitangent = Vec3::Normalize(transform.MulDir(state.bitangent));
		}

		return true;
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