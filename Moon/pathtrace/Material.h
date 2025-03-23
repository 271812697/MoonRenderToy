#pragma once
#include <vector>
#include "MathUtil.h"

namespace PathTrace
{
	enum AlphaMode
	{
		Opaque,
		Blend,
		Mask
	};

	enum MediumType
	{
		None,
		Absorb,
		Scatter,
		Emissive
	};

	class Material
	{
	public:
		Material()
		{
			baseColor = Vec3(1.0f, 1.0f, 1.0f);
			//各向异性
			anisotropic = 0.0f;

			emission = Vec3(0.0f, 0.0f, 0.0f);
			// padding1

			metallic = 0.0f;
			roughness = 0.5f;
			subsurface = 0.0f;
			specularTint = 0.0f;

			sheen = 0.0f;
			sheenTint = 0.0f;
			clearcoat = 0.0f;
			clearcoatGloss = 0.0f;

			specTrans = 0.0f;
			ior = 1.5f;
			//介质类型
			mediumType = 0.0f;
			mediumDensity = 0.0f;

			mediumColor = Vec3(1.0f, 1.0f, 1.0f);
			//各向同性
			mediumAnisotropy = 0.0f;

			baseColorTexId = -1.0f;
			metallicRoughnessTexID = -1.0f;
			normalmapTexID = -1.0f;
			emissionmapTexID = -1.0f;

			opacity = 1.0f;
			alphaMode = 0.0f;
			alphaCutoff = 0.0f;
			// padding2
		};

		Vec3 baseColor;
		float anisotropic;

		Vec3 emission;
		float padding1;

		float metallic;
		float roughness;
		float subsurface;
		float specularTint;

		float sheen;
		float sheenTint;
		float clearcoat;
		float clearcoatGloss;

		float specTrans;
		float ior;
		float mediumType;
		float mediumDensity;

		Vec3 mediumColor;
		float mediumAnisotropy;

		float baseColorTexId;
		float metallicRoughnessTexID;
		float normalmapTexID;
		float emissionmapTexID;

		float opacity;
		float alphaMode;
		float alphaCutoff;
		float padding2;
	};
}
