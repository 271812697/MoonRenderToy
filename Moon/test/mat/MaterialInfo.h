#pragma once

#include "test/DriverEnums.h"//<backend/DriverEnums.h>

#include "MaterialEnums.h"//<filament/>

#include "BufferInterfaceBlock.h"//<private/filament/>
#include "SamplerInterfaceBlock.h"//<private/filament/>
#include "test/utils/FixedCapacityVector.h"

namespace TEST {



	struct  MaterialInfo {
		bool isLit;
		bool hasDoubleSidedCapability;
		bool hasExternalSamplers;
		bool has3dSamplers;
		bool hasShadowMultiplier;
		bool hasTransparentShadow;
		bool specularAntiAliasing;
		bool clearCoatIorChange;
		bool flipUV;
		bool multiBounceAO;
		bool multiBounceAOSet;
		bool specularAOSet;
		bool hasCustomSurfaceShading;
		bool useLegacyMorphing;
		bool instanced;
		bool vertexDomainDeviceJittered;
		bool userMaterialHasCustomDepth;
		int stereoscopicEyeCount;
		SpecularAmbientOcclusion specularAO;
		RefractionMode refractionMode;
		RefractionType refractionType;
		ReflectionMode reflectionMode;
		AttributeBitset requiredAttributes;
		BlendingMode blendingMode;
		BlendingMode postLightingBlendingMode;
		Shading shading;
		BufferInterfaceBlock uib;
		SamplerInterfaceBlock sib;

		ShaderQuality quality;


		using BufferContainer = utils::FixedCapacityVector<BufferInterfaceBlock const*>;
		BufferContainer buffers{ BufferContainer::with_capacity(MAX_SSBO_COUNT) };
	};

}
