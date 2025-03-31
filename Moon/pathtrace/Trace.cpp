#include "Trace.h"
#include "RendererOptions.h"
#include "Scene.h"
#include "Texture.h"
#include "Camera.h"
#include "stb_Image/stb_image_write.h"
#include <cmath>

namespace PathTrace
{
	extern RenderOptions renderOptions;
	extern Scene* scene;
	float SmithG(float NDotV, float alphaG)
	{
		float a = alphaG * alphaG;
		float b = NDotV * NDotV;
		return (2.0 * NDotV) / (NDotV + sqrt(a + b - a * b));
	}
	//The Normal Distrubtion function
	float GTR1(float NDotH, float a)
	{
		if (a >= 1.0)
			return INV_PI;
		float a2 = a * a;
		float t = 1.0 + (a2 - 1.0) * NDotH * NDotH;
		return (a2 - 1.0) / (PI * log(a2) * t);
	}

	float SmithGAniso(float NDotV, float VDotX, float VDotY, float ax, float ay)
	{
		float a = VDotX * ax;
		float b = VDotY * ay;
		float c = NDotV;
		return (2.0 * NDotV) / (NDotV + sqrt(a * a + b * b + c * c));
	}
	//Consideration of Anisotropic Normal Distribution Equation
	float GTR2Aniso(float NDotH, float HDotX, float HDotY, float ax, float ay)
	{
		float a = HDotX / ax;
		float b = HDotY / ay;
		float c = a * a + b * b + NDotH * NDotH;
		return 1.0 / (PI * ax * ay * c * c);
	}
	Vec3 EvalMicrofacetReflection(const Material& mat, const Vec3& V, const Vec3& L, const Vec3& H, const Vec3& F, float& pdf)
	{
		pdf = 0.0;
		if (L.z <= 0.0)
			return Vec3(0.0);

		float D = GTR2Aniso(H.z, H.x, H.y, mat.ax, mat.ay);
		float G1 = SmithGAniso(abs(V.z), V.x, V.y, mat.ax, mat.ay);
		float G2 = G1 * SmithGAniso(abs(L.z), L.x, L.y, mat.ax, mat.ay);

		pdf = G1 * D / (4.0 * V.z);
		return F * D * G2 / (4.0 * L.z * V.z);
	}
	float DielectricFresnel(float cosThetaI, float eta)
	{
		float sinThetaTSq = eta * eta * (1.0f - cosThetaI * cosThetaI);

		// Total internal reflection
		if (sinThetaTSq > 1.0)
			return 1.0;

		float cosThetaT = sqrt(std::max(1.0 - sinThetaTSq, 0.0));

		float rs = (eta * cosThetaT - cosThetaI) / (eta * cosThetaT + cosThetaI);
		float rp = (eta * cosThetaI - cosThetaT) / (eta * cosThetaI + cosThetaT);

		return 0.5f * (rs * rs + rp * rp);
	}
	float SchlickWeight(float u)
	{
		float m = clamp(1.0 - u, 0.0, 1.0);
		float m2 = m * m;
		return m2 * m2 * m;
	}
	Vec3 EvalDisneyDiffuse(const Material& mat, const Vec3& Csheen, const Vec3& V, const Vec3& L, const Vec3& H, float& pdf)
	{
		pdf = 0.0;
		if (L.z <= 0.0)
			return Vec3(0.0);

		float LDotH = Vec3::Dot(L, H);

		float Rr = 2.0 * mat.roughness * LDotH * LDotH;

		// Diffuse
		float FL = SchlickWeight(L.z);
		float FV = SchlickWeight(V.z);
		float Fretro = Rr * (FL + FV + FL * FV * (Rr - 1.0));
		float Fd = (1.0 - 0.5 * FL) * (1.0 - 0.5 * FV);

		// Fake subsurface
		float Fss90 = 0.5 * Rr;
		float Fss = mix(1.0, Fss90, FL) * mix(1.0, Fss90, FV);
		float ss = 1.25 * (Fss * (1.0 / (L.z + V.z) - 0.5) + 0.5);

		// Sheen
		float FH = SchlickWeight(LDotH);
		Vec3 Fsheen = FH * mat.sheen * Csheen;

		pdf = L.z * INV_PI;
		return INV_PI * mat.baseColor * mix(Fd + Fretro, ss, mat.subsurface) + Fsheen;
	}


	void TintColors(Material mat, float eta, float& F0, Vec3& Csheen, Vec3& Cspec0)
	{
		float lum = Luminance(mat.baseColor.x, mat.baseColor.y, mat.baseColor.z);
		Vec3 ctint = lum > 0.0 ? mat.baseColor / lum : Vec3(1.0);

		F0 = (1.0 - eta) / (1.0 + eta);
		F0 *= F0;

		Cspec0 = F0 * mix(Vec3(1.0), ctint, mat.specularTint);
		Csheen = mix(Vec3(1.0), ctint, mat.sheenTint);
	}
	void Onb(const Vec3& N, Vec3& T, Vec3& B)
	{
		Vec3 up = abs(N.z) < 0.9999999 ? Vec3(0, 0, 1) : Vec3(1, 0, 0);
		T = Vec3::Normalize(Vec3::Cross(up, N));
		B = Vec3::Cross(N, T);
	}
	Vec3 ToLocal(Vec3 X, Vec3 Y, Vec3 Z, Vec3 V)
	{
		return Vec3(Vec3::Dot(V, X), Vec3::Dot(V, Y), Vec3::Dot(V, Z));
	}
	Vec3 EvalClearcoat(const Material& mat, const Vec3& V, const Vec3& L, const Vec3& H, float& pdf)
	{
		pdf = 0.0;
		if (L.z <= 0.0)
			return Vec3(0.0);

		float VDotH = Vec3::Dot(V, H);

		float F = mix(0.04, 1.0, SchlickWeight(VDotH));
		float D = GTR1(H.z, mix(0.1, 0.001, mat.clearcoatGloss));
		float G = SmithG(L.z, 0.25) * SmithG(V.z, 0.25);
		float jacobian = 1.0 / (4.0 * VDotH);

		pdf = D * H.z * jacobian;
		return Vec3(F) * D * G;
	}
	//to query BVH for Ray r to konw whether if r hit the scene
	bool AnyHit(Ray r, float maxDist)
	{
		if (renderOptions.optLight) {
			// Intersect Emitters
			for (int i = 0; i < scene->lights.size(); i++)
			{
				// Fetch light Data
				Vec3 position = scene->lights[i].position;
				Vec3 emission = scene->lights[i].emission;
				Vec3 u = scene->lights[i].u;
				Vec3 v = scene->lights[i].v;
				//Vec3 params = texelFetch(lightsTex, ivec2(i * 5 + 4, 0), 0).xyz;
				float radius = scene->lights[i].radius;
				float area = scene->lights[i].area;
				float type = scene->lights[i].type;

				// Intersect rectangular area light
				if (type == QUAD_LIGHT)
				{
					Vec3 normal = Vec3::Normalize(Vec3::Cross(u, v));
					Vec4 plane = Vec4(normal, Vec3::Dot(normal, position));
					u *= 1.0f / Vec3::Dot(u, u);
					v *= 1.0f / Vec3::Dot(v, v);

					float d = RectIntersect(position, u, v, plane, r);
					if (d > 0.0 && d < maxDist)
						return true;
				}

				// Intersect spherical area light
				if (type == SPHERE_LIGHT)
				{
					float d = SphereIntersect(radius, position, r);
					if (d > 0.0 && d < maxDist)
						return true;
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
		int currMatID = -1;
		if (renderOptions.optAlphaTest && !renderOptions.optMedium) {
			currMatID = 0;
		}

		bool BLAS = false;

		Ray rTrans = { r.origin,r.direction };


		while (index != -1)
		{
			Vec3 LRLeaf = scene->bvhTranslator.nodes[index].LRLeaf;;

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

					Vec3 e0 = v1.xyz() - v0.xyz();
					Vec3 e1 = v2.xyz() - v0.xyz();
					Vec3 pv = Vec3::Cross(rTrans.direction, e1);
					float det = Vec3::Dot(e0, pv);

					Vec3 tv = rTrans.origin - v0.xyz();
					Vec3 qv = Vec3::Cross(tv, e0);

					Vec4 uvt;
					uvt.x = Vec3::Dot(tv, pv);
					uvt.y = Vec3::Dot(rTrans.direction, qv);
					uvt.z = Vec3::Dot(e1, qv);
					uvt.x = uvt.x / det;
					uvt.y = uvt.y / det;
					uvt.z = uvt.z / det;
					uvt.w = 1.0 - uvt.x - uvt.y;
					//scene->normalsUVY[scene->vertIndices[leftIndex + i].x].w;
					if (uvt.x >= 0 && uvt.y >= 0 && uvt.z >= 0 && uvt.w >= 0 && uvt.z < maxDist)
					{
						if (renderOptions.optAlphaTest && !renderOptions.optMedium) {
							Vec2 t0 = Vec2(v0.w, scene->normalsUVY[scene->vertIndices[leftIndex + i].x].w);
							Vec2 t1 = Vec2(v1.w, scene->normalsUVY[scene->vertIndices[leftIndex + i].y].w);
							Vec2 t2 = Vec2(v2.w, scene->normalsUVY[scene->vertIndices[leftIndex + i].z].w);

							//clamp
							Vec2 texCoord = t0 * uvt.w + t1 * uvt.x + t2 * uvt.y;
							float alpha = scene->textures[scene->materials[currMatID].baseColorTexId]->Sample(texCoord.x, texCoord.y).w;
							//float alpha = texture(textureMapsArrayTex, Vec3(texCoord, texIDs.x)).a;

							float opacity = scene->materials[currMatID].opacity;
							int alphaMode = int(scene->materials[currMatID].alphaMode);
							float alphaCutoff = scene->materials[currMatID].alphaCutoff;
							opacity *= alpha;

							// Ignore intersection and continue ray based on alpha test
							if (!((alphaMode == ALPHA_MODE_MASK && opacity < alphaCutoff) ||
								(alphaMode == ALPHA_MODE_BLEND && rand() > opacity)))
								return true;
						}
						else
						{
							return true;
						}

					}

				}
			}
			else if (leaf < 0) // Leaf node of TLAS
			{

				Mat4 transform = scene->transforms[-leaf - 1];

				auto tempMat = transform.Inverse();
				rTrans.origin = tempMat.MulPoint(r.origin);
				rTrans.direction = tempMat.MulDir(r.direction);
				// Add a marker. We'll return to this spot after we've traversed the entire BLAS
				stack[ptr++] = -1;

				index = leftIndex;
				BLAS = true;
				if (renderOptions.optAlphaTest && !renderOptions.optMedium) {
					currMatID = rightIndex;
				}
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

		return false;
	}
	bool ClosestHit(Ray r, State& state, LightSampleRec& lightSample) {
		float t = INF;
		float d;
		if (renderOptions.optLight) {
			if ((renderOptions.hideEmitters && state.depth > 0) || !renderOptions.hideEmitters) {
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


		Ray rTrans = { r.origin,r.direction };


		while (index != -1)
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
						state.matID = currMatID;
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
			float wi;
			state.texCoord.x = modf(state.texCoord.x, &wi);
			state.texCoord.y = modf(state.texCoord.y, &wi);
			state.texCoord.x = state.texCoord.x + 1;
			state.texCoord.y = state.texCoord.y + 1;
			state.texCoord.x = modf(state.texCoord.x, &wi);
			state.texCoord.y = modf(state.texCoord.y, &wi);
			//state.texCoord.x = modf(state.texCoord.x, &wi);

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
	Vec3 EvalMicrofacetRefraction(const Material& mat, float eta, const Vec3& V, const Vec3& L, const Vec3& H, const Vec3& F, float& pdf)
	{
		pdf = 0.0;
		if (L.z >= 0.0)
			return Vec3(0.0);

		float LDotH = Vec3::Dot(L, H);
		float VDotH = Vec3::Dot(V, H);

		float D = GTR2Aniso(H.z, H.x, H.y, mat.ax, mat.ay);
		float G1 = SmithGAniso(abs(V.z), V.x, V.y, mat.ax, mat.ay);
		float G2 = G1 * SmithGAniso(abs(L.z), L.x, L.y, mat.ax, mat.ay);
		float denom = LDotH + VDotH * eta;
		denom *= denom;
		float eta2 = eta * eta;
		float jacobian = abs(LDotH) / denom;

		pdf = G1 * std::max(0.0f, VDotH) * D * jacobian / V.z;
		return pow(mat.baseColor, Vec3(0.5)) * (-F + 1.0) * D * G2 * abs(VDotH) * jacobian * eta2 / abs(L.z * V.z);
	}

	Vec4 EvalEnvMap(const Ray& r) {
		float theta = acos(clamp(r.direction.y, -1.0, 1.0));

		Vec2 uv = Vec2((PI + atan2(r.direction.z, r.direction.x)) * INV_TWO_PI, theta * INV_PI) + Vec2(renderOptions.envMapRot / 360.0f, 0.0);
		Vec3 color = scene->envMap->Sample(uv.x, uv.y);
		float pdf = Luminance(color.x, color.y, color.z) / scene->envMap->totalSum;
		return Vec4(color, (pdf * scene->envMap->width * scene->envMap->height) / (TWO_PI * PI * sin(theta)));
	}
	void GetMaterial(State& state, const Ray& r) {
		Material mat = scene->materials[state.matID];

		Medium medium;
		//Base Color Map
		if (mat.baseColorTexId >= 0) {
			Vec4 col = scene->textures[mat.baseColorTexId]->Sample(state.texCoord.x, state.texCoord.y);
			mat.baseColor *= pow(col.xyz(), Vec3(2.2));;
			mat.opacity *= col.w;
		}

		//Metallic Roughness Map
		if (mat.metallicRoughnessTexID >= 0) {
			Vec3 matRgh = scene->textures[mat.metallicRoughnessTexID]->Sample(state.texCoord.x, state.texCoord.y);
			mat.metallic = matRgh.z;
			mat.roughness = std::max(matRgh.y * matRgh.y, 0.001f);
		}

		if (mat.normalmapTexID >= 0) {
			Vec3 texNormal = scene->textures[mat.normalmapTexID]->Sample(state.texCoord.x, state.texCoord.y).xyz();
			if (renderOptions.openglNormalMap) {
				texNormal.y = 1.0 - texNormal.y;
			}
			texNormal = Vec3::Normalize(texNormal * 2 - 1.0f);
			Vec3 origNormal = state.normal;
			state.normal = Vec3::Normalize(state.tangent * texNormal.x + state.bitangent * texNormal.y + state.normal * texNormal.z);
			state.ffnormal = Vec3::Dot(origNormal, r.direction) <= 0.0 ? state.normal : -state.normal;
		}

		if (renderOptions.enableRoughnessMollification) {
			if (state.depth > 0)
				mat.roughness = std::max(state.mat.roughness * renderOptions.roughnessMollificationAmt, mat.roughness);
		}

		if (mat.emissionmapTexID >= 0) {
			mat.emission = pow(scene->textures[mat.emissionmapTexID]->Sample(state.texCoord.x, state.texCoord.y).xyz(), Vec3(2.2));
		}

		float aspect = sqrt(1.0 - mat.anisotropic * 0.9);
		mat.ax = std::max(0.001f, mat.roughness / aspect);
		mat.ay = std::max(0.001f, mat.roughness * aspect);

		state.mat = mat;
		state.eta = Vec3::Dot(r.direction, state.normal) < 0.0 ? (1.0 / mat.ior) : mat.ior;
	}
	Vec3 UniformSampleHemisphere(float r1, float r2)
	{
		float r = sqrt(std::max(0.0, 1.0 - r1 * r1));
		float phi = TWO_PI * r2;
		return Vec3(r * cos(phi), r * sin(phi), r1);
	}

	Vec3 EvalTransmittance(Ray r)
	{
		LightSampleRec lightSample;
		State state;
		Vec3 transmittance = Vec3(1.0);

		for (int depth = 0; depth < renderOptions.maxDepth; depth++)
		{
			bool hit = ClosestHit(r, state, lightSample);

			// If no hit (environment map) or if ray hit a light source then return transmittance
			if (!hit || state.isEmitter)
				break;

			// TODO: Get only parameters that are needed to calculate transmittance
			GetMaterial(state, r);

			bool alphatest = (state.mat.alphaMode == ALPHA_MODE_MASK && state.mat.opacity < state.mat.alphaCutoff) || (state.mat.alphaMode == ALPHA_MODE_BLEND && rand() > state.mat.opacity);
			bool refractive = (1.0 - state.mat.metallic) * state.mat.specTrans > 0.0;

			// Refraction is ignored (Not physically correct but helps with sampling lights from inside refractive objects)
			if (hit && !(alphatest || refractive))
				return Vec3(0.0);

			// Evaluate transmittance
			if (Vec3::Dot(r.direction, state.normal) > 0 && state.medium.type != MEDIUM_NONE)
			{
				Vec3 color = state.medium.type == MEDIUM_ABSORB ? Vec3(1.0) - state.medium.color : Vec3(1.0);
				transmittance *= exp(-color * state.medium.density * state.hitDist);
			}

			// Move ray origin to hit point
			r.origin = state.fhp + r.direction * EPS;
		}
		return transmittance;
	}
	Vec4 SampleEnvMap(Vec3& color) {
		Vec2 uv;
		color = scene->envMap->Sample(uniform_float(), &uv);
		float pdf = Luminance(color.x, color.y, color.z) / scene->envMap->totalSum;

		uv.x -= scene->renderOptions.envMapRot / 360.0f;
		float phi = uv.x * TWO_PI;
		float theta = uv.y * PI;


		if (sin(theta) == 0.0)
			pdf = 0.0;
		return Vec4(-sin(theta) * cos(phi), cos(theta), -sin(theta) * sin(phi), (pdf * scene->envMap->width * scene->envMap->height) / (TWO_PI * PI * sin(theta)));
	}
	Vec3 DisneyEval(const State& state, Vec3 V, const Vec3& N, Vec3 L, float& pdf);
	void SampleSphereLight(const Light& light, const Vec3& scatterPos, LightSampleRec& lightSample)
	{
		float r1 = rand();
		float r2 = rand();

		Vec3 sphereCentertoSurface = scatterPos - light.position;
		float distToSphereCenter = Vec3::Length(sphereCentertoSurface);
		Vec3 sampledDir;

		// TODO: Fix this. Currently assumes the light will be hit only from the outside
		sphereCentertoSurface /= distToSphereCenter;
		sampledDir = UniformSampleHemisphere(r1, r2);
		Vec3 T, B;
		Onb(sphereCentertoSurface, T, B);
		sampledDir = T * sampledDir.x + B * sampledDir.y + sphereCentertoSurface * sampledDir.z;

		Vec3 lightSurfacePos = light.position + sampledDir * light.radius;

		lightSample.direction = lightSurfacePos - scatterPos;
		lightSample.dist = Vec3::Length(lightSample.direction);
		float distSq = lightSample.dist * lightSample.dist;

		lightSample.direction /= lightSample.dist;
		lightSample.normal = Vec3::Normalize(lightSurfacePos - light.position);
		lightSample.emission = light.emission * float(scene->lights.size());
		lightSample.pdf = distSq / (light.area * 0.5 * abs(Vec3::Dot(lightSample.normal, lightSample.direction)));
	}

	void SampleRectLight(const Light& light, const Vec3& scatterPos, LightSampleRec& lightSample)
	{
		float r1 = rand();
		float r2 = rand();

		Vec3 lightSurfacePos = light.position + light.u * r1 + light.v * r2;
		lightSample.direction = lightSurfacePos - scatterPos;
		lightSample.dist = Vec3::Length(lightSample.direction);
		float distSq = lightSample.dist * lightSample.dist;
		lightSample.direction /= lightSample.dist;
		lightSample.normal = Vec3::Normalize(Vec3::Cross(light.u, light.v));
		lightSample.emission = light.emission * float(scene->lights.size());
		lightSample.pdf = distSq / (light.area * abs(Vec3::Dot(lightSample.normal, lightSample.direction)));
	}

	void SampleDistantLight(const Light& light, const Vec3& scatterPos, LightSampleRec& lightSample)
	{
		lightSample.direction = Vec3::Normalize(light.position - Vec3(0.0));
		lightSample.normal = Vec3::Normalize(scatterPos - light.position);
		lightSample.emission = light.emission * float(scene->lights.size());
		lightSample.dist = INF;
		lightSample.pdf = 1.0;
	}

	void SampleOneLight(const Light& light, const Vec3& scatterPos, LightSampleRec& lightSample)
	{
		int type = int(light.type);

		if (type == QUAD_LIGHT)
			SampleRectLight(light, scatterPos, lightSample);
		else if (type == SPHERE_LIGHT)
			SampleSphereLight(light, scatterPos, lightSample);
		else
			SampleDistantLight(light, scatterPos, lightSample);
	}
	Vec3 DirectLight(const Ray& r, const State& state, bool isSurface) {
		Vec3 Ld = Vec3(0.0);
		Vec3 Li = Vec3(0.0);
		Vec3 scatterPos = state.fhp + state.normal * EPS;

		ScatterSampleRec scatterSample;
		if (renderOptions.enableEnvMap && !renderOptions.enableUniformLight) {
			Vec3 color;
			Vec4 dirPdf = SampleEnvMap(Li);
			Vec3 lightDir = dirPdf.xyz();
			float lightPdf = dirPdf.w;
			Ray shadowRay = Ray(scatterPos, lightDir);
			if (renderOptions.optMedium && renderOptions.enableVolumeMIS) {
				Li *= EvalTransmittance(shadowRay);
				if (isSurface)
					scatterSample.f = DisneyEval(state, -r.direction, state.ffnormal, lightDir, scatterSample.pdf);
				else
				{
					float p = PhaseHG(Vec3::Dot(-r.direction, lightDir), state.medium.anisotropy);
					scatterSample.f = Vec3(p);
					scatterSample.pdf = p;
				}

				if (scatterSample.pdf > 0.0)
				{
					float misWeight = PowerHeuristic(lightPdf, scatterSample.pdf);
					if (misWeight > 0.0)
						Ld += misWeight * Li * scatterSample.f * scene->renderOptions.envMapIntensity / lightPdf;
				}
			}
			else
			{
				// If there are no volumes in the scene then use a simple binary hit test
				bool inShadow = AnyHit(shadowRay, INF - EPS);

				if (!inShadow)
				{
					scatterSample.f = DisneyEval(state, -r.direction, state.ffnormal, lightDir, scatterSample.pdf);

					if (scatterSample.pdf > 0.0)
					{

						float misWeight = PowerHeuristic(lightPdf, scatterSample.pdf);
						if (misWeight > 0.0)
							Ld += misWeight * Li * scatterSample.f * renderOptions.envMapIntensity / lightPdf;
					}
				}
			}
		}
		// Analytic Lights
		if (renderOptions.optLight) {
			LightSampleRec lightSample;
			Light light;

			//Pick a light to sample
			int index = int(uniform_float() * float(scene->lights.size())) * 5;

			// Fetch light Data


			light = scene->lights[index];
			SampleOneLight(light, scatterPos, lightSample);
			Li = lightSample.emission;
			if (Vec3::Dot(lightSample.direction, lightSample.normal)) {
				Ray shadowRay = Ray(scatterPos, lightSample.direction);
				if (renderOptions.optMedium && renderOptions.enableVolumeMIS) {
					Li *= EvalTransmittance(shadowRay);

					if (isSurface)
						scatterSample.f = DisneyEval(state, -r.direction, state.ffnormal, lightSample.direction, scatterSample.pdf);
					else
					{
						float p = PhaseHG(Vec3::Dot(-r.direction, lightSample.direction), state.medium.anisotropy);
						scatterSample.f = Vec3(p);
						scatterSample.pdf = p;
					}

					float misWeight = 1.0;
					if (light.area > 0.0) // No MIS for distant light
						misWeight = PowerHeuristic(lightSample.pdf, scatterSample.pdf);

					if (scatterSample.pdf > 0.0)
						Ld += misWeight * scatterSample.f * Li / lightSample.pdf;

				}
				else
				{
					// If there are no volumes in the scene then use a simple binary hit test
					bool inShadow = AnyHit(shadowRay, lightSample.dist - EPS);

					if (!inShadow)
					{
						scatterSample.f = DisneyEval(state, -r.direction, state.ffnormal, lightSample.direction, scatterSample.pdf);

						float misWeight = 1.0;
						if (light.area > 0.0) // No MIS for distant light
							misWeight = PowerHeuristic(lightSample.pdf, scatterSample.pdf);

						if (scatterSample.pdf > 0.0)
							Ld += misWeight * Li * scatterSample.f / lightSample.pdf;
					}
				}

			}
		}


		return Ld;

	}
	Vec3 SampleHG(const Vec3& V, float g, float r1, float r2)
	{
		float cosTheta;

		if (abs(g) < 0.001)
			cosTheta = 1 - 2 * r2;
		else
		{
			float sqrTerm = (1 - g * g) / (1 + g - 2 * g * r2);
			cosTheta = -(1 + g * g - sqrTerm * sqrTerm) / (2 * g);
		}

		float phi = r1 * TWO_PI;
		float sinTheta = clamp(sqrt(1.0 - (cosTheta * cosTheta)), 0.0, 1.0);
		float sinPhi = sin(phi);
		float cosPhi = cos(phi);

		Vec3 v1, v2;
		Onb(V, v1, v2);

		return sinTheta * cosPhi * v1 + sinTheta * sinPhi * v2 + cosTheta * V;
	}
	Vec3 DisneyEval(const State& state, Vec3 V, const Vec3& N, Vec3 L, float& pdf)
	{
		pdf = 0.0;
		Vec3 f = Vec3(0.0);

		// TODO: Tangent and bitangent should be calculated from mesh (provided, the mesh has proper uvs)
		Vec3 T, B;
		Onb(N, T, B);

		// Transform to shading space to simplify operations (NDotL = L.z; NDotV = V.z; NDotH = H.z)
		V = ToLocal(T, B, N, V);
		L = ToLocal(T, B, N, L);

		Vec3 H;
		if (L.z > 0.0)
			H = Vec3::Normalize(L + V);
		else
			H = Vec3::Normalize(L + V * state.eta);

		if (H.z < 0.0)
			H = -H;

		// Tint colors
		Vec3 Csheen, Cspec0;
		float F0;
		TintColors(state.mat, state.eta, F0, Csheen, Cspec0);

		// Model weights
		float dielectricWt = (1.0 - state.mat.metallic) * (1.0 - state.mat.specTrans);
		float metalWt = state.mat.metallic;
		float glassWt = (1.0 - state.mat.metallic) * state.mat.specTrans;

		// Lobe probabilities
		float schlickWt = SchlickWeight(V.z);

		float diffPr = dielectricWt * Luminance(state.mat.baseColor.x, state.mat.baseColor.y, state.mat.baseColor.z);
		float dielectricPr = dielectricWt * Luminance(mix(Cspec0, Vec3(1.0), schlickWt));
		float metalPr = metalWt * Luminance(mix(state.mat.baseColor, Vec3(1.0), schlickWt));
		float glassPr = glassWt;
		float clearCtPr = 0.25 * state.mat.clearcoat;

		// Normalize probabilities
		float invTotalWt = 1.0 / (diffPr + dielectricPr + metalPr + glassPr + clearCtPr);
		diffPr *= invTotalWt;
		dielectricPr *= invTotalWt;
		metalPr *= invTotalWt;
		glassPr *= invTotalWt;
		clearCtPr *= invTotalWt;

		bool reflect = L.z * V.z > 0;

		float tmpPdf = 0.0;
		float VDotH = abs(Vec3::Dot(V, H));

		// Diffuse
		if (diffPr > 0.0 && reflect)
		{
			f += EvalDisneyDiffuse(state.mat, Csheen, V, L, H, tmpPdf) * dielectricWt;
			pdf += tmpPdf * diffPr;
		}

		// Dielectric Reflection
		if (dielectricPr > 0.0 && reflect)
		{
			// Normalize for interpolating based on Cspec0
			float F = (DielectricFresnel(VDotH, 1.0 / state.mat.ior) - F0) / (1.0 - F0);

			f += EvalMicrofacetReflection(state.mat, V, L, H, mix(Cspec0, Vec3(1.0), F), tmpPdf) * dielectricWt;
			pdf += tmpPdf * dielectricPr;
		}

		// Metallic Reflection
		if (metalPr > 0.0 && reflect)
		{
			// Tinted to base color
			Vec3 F = mix(state.mat.baseColor, Vec3(1.0), SchlickWeight(VDotH));

			f += EvalMicrofacetReflection(state.mat, V, L, H, F, tmpPdf) * metalWt;
			pdf += tmpPdf * metalPr;
		}

		// Glass/Specular BSDF
		if (glassPr > 0.0)
		{
			// Dielectric fresnel (achromatic)
			float F = DielectricFresnel(VDotH, state.eta);

			if (reflect)
			{
				f += EvalMicrofacetReflection(state.mat, V, L, H, Vec3(F), tmpPdf) * glassWt;
				pdf += tmpPdf * glassPr * F;
			}
			else
			{
				f += EvalMicrofacetRefraction(state.mat, state.eta, V, L, H, Vec3(F), tmpPdf) * glassWt;
				pdf += tmpPdf * glassPr * (1.0 - F);
			}
		}

		// Clearcoat
		if (clearCtPr > 0.0 && reflect)
		{
			f += EvalClearcoat(state.mat, V, L, H, tmpPdf) * 0.25 * state.mat.clearcoat;
			pdf += tmpPdf * clearCtPr;
		}

		return f * abs(L.z);
	}
	Vec3 CosineSampleHemisphere(float r1, float r2)
	{
		Vec3 dir;
		float r = sqrt(r1);
		float phi = TWO_PI * r2;
		dir.x = r * cos(phi);
		dir.y = r * sin(phi);
		dir.z = sqrt(std::max(0.0, 1.0 - dir.x * dir.x - dir.y * dir.y));
		return dir;
	}
	//Sample a H
	Vec3 SampleGGXVNDF(const Vec3& V, float ax, float ay, float r1, float r2)
	{
		Vec3 Vh = Vec3::Normalize(Vec3(ax * V.x, ay * V.y, V.z));

		float lensq = Vh.x * Vh.x + Vh.y * Vh.y;
		Vec3 T1 = lensq > 0 ? Vec3(-Vh.y, Vh.x, 0) * 1.0f / sqrt(lensq) : Vec3(1, 0, 0);
		Vec3 T2 = Vec3::Cross(Vh, T1);

		float r = sqrt(r1);
		float phi = 2.0 * PI * r2;
		float t1 = r * cos(phi);
		float t2 = r * sin(phi);
		float s = 0.5 * (1.0 + Vh.z);
		t2 = (1.0 - s) * sqrt(1.0 - t1 * t1) + s * t2;

		Vec3 Nh = t1 * T1 + t2 * T2 + sqrt(std::max(0.0, 1.0 - t1 * t1 - t2 * t2)) * Vh;

		return Vec3::Normalize(Vec3(ax * Nh.x, ay * Nh.y, std::max(0.0f, Nh.z)));
	}
	Vec3 ToWorld(const Vec3& X, const Vec3& Y, const Vec3& Z, const Vec3& V)
	{
		return V.x * X + V.y * Y + V.z * Z;
	}
	//this is used to sample a vec3 in hemisphere with roughness importance 
//the roughness can decide the angle of cosTheta which limits the range of sample
//when roughness is equal to 1,cosTheta  is pi/2 ,and 0 -> 0
//this can be useful to query the direction of specular light 
	Vec3 SampleGTR1(float rgh, float r1, float r2)
	{
		float a = std::max(0.001f, rgh);
		float a2 = a * a;

		float phi = r1 * TWO_PI;

		float cosTheta = sqrt((1.0f - std::pow(a2, 1.0f - r2)) / (1.0f - a2));
		float sinTheta = clamp(sqrt(1.0 - (cosTheta * cosTheta)), 0.0, 1.0);
		float sinPhi = sin(phi);
		float cosPhi = cos(phi);

		return Vec3(sinTheta * cosPhi, sinTheta * sinPhi, cosTheta);
	}
	Vec3 DisneySample(const State& state, Vec3 V, Vec3 N, Vec3& L, float& pdf)
	{
		pdf = 0.0;

		float r1 = rand();
		float r2 = rand();

		// TODO: Tangent and bitangent should be calculated from mesh (provided, the mesh has proper uvs)
		Vec3 T, B;
		Onb(N, T, B);

		// Transform to shading space to simplify operations (NDotL = L.z; NDotV = V.z; NDotH = H.z)
		V = ToLocal(T, B, N, V);

		// Tint colors
		Vec3 Csheen, Cspec0;
		float F0;
		TintColors(state.mat, state.eta, F0, Csheen, Cspec0);

		// Model weights
		float dielectricWt = (1.0 - state.mat.metallic) * (1.0 - state.mat.specTrans);
		float metalWt = state.mat.metallic;
		float glassWt = (1.0 - state.mat.metallic) * state.mat.specTrans;

		// Lobe probabilities
		float schlickWt = SchlickWeight(V.z);

		float diffPr = dielectricWt * Luminance(state.mat.baseColor);
		float dielectricPr = dielectricWt * Luminance(mix(Cspec0, Vec3(1.0), schlickWt));
		float metalPr = metalWt * Luminance(mix(state.mat.baseColor, Vec3(1.0), schlickWt));
		float glassPr = glassWt;
		float clearCtPr = 0.25 * state.mat.clearcoat;

		// Normalize probabilities
		float invTotalWt = 1.0 / (diffPr + dielectricPr + metalPr + glassPr + clearCtPr);
		diffPr *= invTotalWt;
		dielectricPr *= invTotalWt;
		metalPr *= invTotalWt;
		glassPr *= invTotalWt;
		clearCtPr *= invTotalWt;

		// CDF of the sampling probabilities
		float cdf[5];
		cdf[0] = diffPr;
		cdf[1] = cdf[0] + dielectricPr;
		cdf[2] = cdf[1] + metalPr;
		cdf[3] = cdf[2] + glassPr;
		cdf[4] = cdf[3] + clearCtPr;

		// Sample a lobe based on its importance
		float r3 = rand();

		if (r3 < cdf[0]) // Diffuse
		{
			L = CosineSampleHemisphere(r1, r2);
		}
		else if (r3 < cdf[2]) // Dielectric + Metallic reflection
		{
			Vec3 H = SampleGGXVNDF(V, state.mat.ax, state.mat.ay, r1, r2);

			if (H.z < 0.0)
				H = -H;

			L = Vec3::Normalize(reflect(-V, H));
		}
		else if (r3 < cdf[3]) // Glass
		{
			Vec3 H = SampleGGXVNDF(V, state.mat.ax, state.mat.ay, r1, r2);
			float F = DielectricFresnel(abs(Vec3::Dot(V, H)), state.eta);

			if (H.z < 0.0)
				H = -H;

			// Rescale random number for reuse
			r3 = (r3 - cdf[2]) / (cdf[3] - cdf[2]);

			// Reflection
			if (r3 < F)
			{
				L = Vec3::Normalize(reflect(-V, H));
			}
			else // Transmission
			{
				L = Vec3::Normalize(refract(-V, H, state.eta));
			}
		}
		else // Clearcoat
		{
			Vec3 H = SampleGTR1(mix(0.1, 0.001, state.mat.clearcoatGloss), r1, r2);

			if (H.z < 0.0)
				H = -H;

			L = Vec3::Normalize(reflect(-V, H));
		}

		L = ToWorld(T, B, N, L);
		V = ToWorld(T, B, N, V);

		return DisneyEval(state, V, N, L, pdf);
	}

	Vec4 Trace(Ray& r)
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
			bool hit = ClosestHit(r, state, lightSample);
			if (!hit) {
				if (renderOptions.enableBackground || renderOptions.transparentBackground) {
					if (state.depth == 0) {
						alpha = 0.0;
					}
				}
				if (!renderOptions.hideEmitters || (renderOptions.hideEmitters && state.depth > 0)) {
					if (renderOptions.enableUniformLight) {
						radiance += renderOptions.uniformLightCol * throughput;
					}
					else if (renderOptions.enableEnvMap) {
						Vec4 envMapColPdf = EvalEnvMap(r);

						float misWeight = 1.0;
						if (state.depth > 0) {
							misWeight = PowerHeuristic(scatterSample.pdf, envMapColPdf.w);
						}
						if (renderOptions.optMedium && !renderOptions.enableVolumeMIS) {//#if defined(OPT_MEDIUM) && !defined(OPT_VOL_MIS)
							if (!surfaceScatter)
								misWeight = 1.0f;
						}

						if (misWeight > 0)
							radiance += misWeight * envMapColPdf.xyz() * throughput * renderOptions.envMapIntensity;

					}

				}
				break;
			}
			GetMaterial(state, r);
			// Gather radiance from emissive objects. Emission from meshes is not importance sampled
			radiance += state.mat.emission * throughput;
			if (renderOptions.optLight) {
				// Gather radiance from light and use scatterSample.pdf from previous bounce for MIS
				if (state.isEmitter) {
					float misWeight = 1.0;
					if (state.depth > 0)
						misWeight = PowerHeuristic(scatterSample.pdf, lightSample.pdf);
					if (renderOptions.optMedium && !renderOptions.enableVolumeMIS) {
						if (!surfaceScatter)
							misWeight = 1.0f;
					}
					radiance += misWeight * lightSample.emission * throughput;
					break;
				}
			}

			// Stop tracing ray if maximum depth was reached
			if (state.depth == renderOptions.maxDepth)
				break;
			if (renderOptions.optMedium) {
				mediumSampled = false;
				surfaceScatter = false;
				// Handle absorption/emission/scattering from medium
				// TODO: Handle light sources placed inside medium
				if (inMedium)
				{
					if (state.medium.type == MEDIUM_ABSORB)
					{

						throughput *= exp(-(-state.medium.color + 1.0) * state.hitDist * state.medium.density);
					}
					else if (state.medium.type == MEDIUM_EMISSIVE)
					{
						radiance += state.medium.color * state.hitDist * state.medium.density * throughput;
					}
					else
					{
						// Sample a distance in the medium
						float scatterDist = std::min(-log(uniform_float()) / state.medium.density, state.hitDist);
						mediumSampled = scatterDist < state.hitDist;

						if (mediumSampled)
						{
							throughput *= state.medium.color;

							// Move ray origin to scattering position
							r.origin += r.direction * scatterDist;
							state.fhp = r.origin;

							// Transmittance Evaluation
							radiance += DirectLight(r, state, false) * throughput;

							// Pick a new direction based on the phase function
							Vec3 scatterDir = SampleHG(-r.direction, state.medium.anisotropy, rand(), rand());
							scatterSample.pdf = PhaseHG(Vec3::Dot(-r.direction, scatterDir), state.medium.anisotropy);
							r.direction = scatterDir;
						}
					}
				}
			}


			// If medium was not sampled then proceed with surface BSDF evaluation
			if ((renderOptions.optMedium && !mediumSampled) || !renderOptions.optMedium)
			{
				if (renderOptions.optAlphaTest) {
					// Ignore intersection and continue ray based on alpha test
					if ((state.mat.alphaMode == ALPHA_MODE_MASK && state.mat.opacity < state.mat.alphaCutoff) ||
						(state.mat.alphaMode == ALPHA_MODE_BLEND && rand() > state.mat.opacity))
					{
						scatterSample.L = r.direction;
						state.depth--;
					}
				}
				else
				{
					surfaceScatter = true;

					// Next event estimation
					radiance += DirectLight(r, state, true) * throughput;

					// Sample BSDF for color and outgoing direction
					scatterSample.f = DisneySample(state, -r.direction, state.ffnormal, scatterSample.L, scatterSample.pdf);
					if (scatterSample.pdf > 0.0)
						throughput *= scatterSample.f / scatterSample.pdf;
					else
						break;
				}
				// Move ray origin to hit point and set direction for next bounce
				r.direction = scatterSample.L;
				r.origin = state.fhp + r.direction * EPS;

				if (renderOptions.optMedium) {
					// Note: Nesting of volumes isn't supported due to lack of a volume stack for performance reasons
					// Ray is in medium only if it is entering a surface containing a medium
					if (Vec3::Dot(r.direction, state.normal) < 0 && state.mat.mediumType != MEDIUM_NONE)
					{
						inMedium = true;
						//state.mat.subsurface
						// Get medium params from the intersected object
						state.medium.type = state.mat.mediumType;
					}
					// FIXME: Objects clipping or inside a medium were shaded incorrectly as inMedium would be set to false.
					// This hack works for now but needs some rethinking
					else if (state.mat.mediumType != MEDIUM_NONE)
						inMedium = false;
				}
			}
			if (renderOptions.enableRR && state.depth >= renderOptions.RRDepth) {
				float q = std::min(std::max(throughput.x, std::max(throughput.y, throughput.z)) + 0.001, 0.95);
				if (uniform_float() > q)
					break;
				throughput /= q;
			}

		}
		//std::cout << "(" << radiance.x << "," << radiance.y << "," << radiance.z << ")" << std::endl;
		return Vec4(radiance, alpha);
	}
	Vec3 ACES(const Vec3& c)
	{
		float a = 2.51f;
		float b = 0.03f;
		float y = 2.43f;
		float d = 0.59f;
		float e = 0.14f;

		return clamp((c * (a * c + b)) / (c * (y * c + d) + e), 0.0, 1.0);
	}

	void TraceScreen(int width, int height) {
		if (height < 500) {
			return;
		}
		int w = 800;
		int h = 800;
		static float* imagesum = nullptr;//
		static unsigned char* image = nullptr;
		if (imagesum == nullptr) {
			imagesum = new float[w * h * 4];
			image = new unsigned char[w * h * 4];
			for (int i = 0; i < w * h * 4; i++) {
				imagesum[i] = 0.0f;
			}

		}
		float* sum = imagesum;
		unsigned char* data = image;
		static int cnt = 1;
		for (int i = 0; i < w; i++) {
			for (int j = 0; j < h; j++) {
				InitRNG(Vec2(i, j), cnt);
				float r1 = 2.0 * uniform_float();
				float r2 = 2.0 * uniform_float();
				Vec2 jitter;
				jitter.x = r1 < 1.0 ? sqrt(r1) - 1.0 : 1.0 - sqrt(2.0 - r1);
				jitter.y = r2 < 1.0 ? sqrt(r2) - 1.0 : 1.0 - sqrt(2.0 - r2);
				jitter.x /= 2.0f * w;
				jitter.y /= 2.0f * h;
				Vec2 dd = Vec2(i * 2.0f / w - 1.0f, j * 2.0f / h - 1.0f) + jitter;

				float scale = tan(scene->camera->fov * 0.5);
				//fov水平方向的张角
				dd.y *= h * 1.0f / w * scale;
				dd.x *= scale;
				Vec3 RayDir = scene->camera->right * dd.x + scene->camera->up * dd.y + scene->camera->forward;
				RayDir = Vec3::Normalize(RayDir);
				Vec3 RayPos = scene->camera->position;
				Ray r = { RayPos ,RayDir };
				Vec3 res = Trace(r).xyz();
				*sum = *sum + res.x;
				*(sum + 1) = *(sum + 1) + res.y;
				*(sum + 2) = *(sum + 2) + res.z;
				res = ACES(Vec3(*(sum) / cnt, *(sum + 1) / cnt, *(sum + 2) / cnt));
				res = pow(res, Vec3(2.2));

				*data++ = res.x * 255.0;
				*data++ = res.y * 255.0;
				*data++ = res.z * 255.0;
				*data++ = 255;
				sum += 4;;
			}
		}


		std::string file = "res_" + std::to_string(cnt++) + ".png";
		stbi_write_png(file.c_str(), w, h, 4, image, 0);

		//delete[] data;
	}
}