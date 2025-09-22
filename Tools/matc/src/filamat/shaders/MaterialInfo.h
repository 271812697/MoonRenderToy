#pragma once
#include <filaflat/backend/DriverEnums.h>
#include <filaflat/MaterialEnums.h>
#include <filaflat/BufferInterfaceBlock.h>
#include <filaflat/SamplerInterfaceBlock.h>
#include <filaflat/ShaderPassInfo.h>


namespace filamat {

using UniformType = filament::backend::UniformType;
using SamplerType = filament::backend::SamplerType;
using CullingMode = filament::backend::CullingMode;

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
    filament::SpecularAmbientOcclusion specularAO;
    filament::RefractionMode refractionMode;
    filament::RefractionType refractionType;
    filament::ReflectionMode reflectionMode;
    filament::BlendingMode blendingMode;
    filament::BlendingMode postLightingBlendingMode;
    filament::Shading shading;
    filament::BufferInterfaceBlock uib;
    filament::SamplerInterfaceBlock sib;
    filament::ShaderPassInfo passInfo;
    filament::ShaderQuality quality;
   


    using BufferContainer = std::vector<filament::BufferInterfaceBlock const*>;
    BufferContainer buffers;
};

}
