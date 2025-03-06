/*
 * Copyright (C) 2015 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "UibGenerator.h"
#include "UibStructs.h"

#include "BufferInterfaceBlock.h"

 //#include <private/filament/EngineEnums.h>
#include "EngineEnums.h"


#include <stdlib.h>

namespace TEST {



	BufferInterfaceBlock const& UibGenerator::get(UibGenerator::Ubo ubo) noexcept {
		assert(ubo != Ubo::MaterialParams);
		switch (ubo) {
		case Ubo::FrameUniforms:
			return getPerViewUib();
		case Ubo::ObjectUniforms:
			return getPerRenderableUib();
		case Ubo::BonesUniforms:
			return getPerRenderableBonesUib();
		case Ubo::MorphingUniforms:
			return getPerRenderableMorphingUib();
		case Ubo::LightsUniforms:
			return getLightsUib();
		case Ubo::ShadowUniforms:
			return getShadowUib();
		case Ubo::FroxelRecordUniforms:
			return getFroxelRecordUib();
		case Ubo::FroxelsUniforms:
			return getFroxelsUib();
		case Ubo::MaterialParams:
			abort();
		}
	}

	UibGenerator::Binding UibGenerator::getBinding(UibGenerator::Ubo ubo) noexcept {
		switch (ubo) {
		case Ubo::FrameUniforms:
			return { +DescriptorSetBindingPoints::PER_VIEW,
					 +PerViewBindingPoints::FRAME_UNIFORMS };
		case Ubo::ObjectUniforms:
			return { +DescriptorSetBindingPoints::PER_RENDERABLE,
					 +PerRenderableBindingPoints::OBJECT_UNIFORMS };
		case Ubo::BonesUniforms:
			return { +DescriptorSetBindingPoints::PER_RENDERABLE,
					 +PerRenderableBindingPoints::BONES_UNIFORMS };
		case Ubo::MorphingUniforms:
			return { +DescriptorSetBindingPoints::PER_RENDERABLE,
					 +PerRenderableBindingPoints::MORPHING_UNIFORMS };
		case Ubo::LightsUniforms:
			return { +DescriptorSetBindingPoints::PER_VIEW,
					 +PerViewBindingPoints::LIGHTS };
		case Ubo::ShadowUniforms:
			return { +DescriptorSetBindingPoints::PER_VIEW,
					 +PerViewBindingPoints::SHADOWS };
		case Ubo::FroxelRecordUniforms:
			return { +DescriptorSetBindingPoints::PER_VIEW,
					 +PerViewBindingPoints::RECORD_BUFFER };
		case Ubo::FroxelsUniforms:
			return { +DescriptorSetBindingPoints::PER_VIEW,
					 +PerViewBindingPoints::FROXEL_BUFFER };
		case Ubo::MaterialParams:
			return { +DescriptorSetBindingPoints::PER_MATERIAL,
					 +PerMaterialBindingPoints::MATERIAL_PARAMS };
		}
	}

	static_assert(CONFIG_MAX_SHADOW_CASCADES == 4,
		"Changing CONFIG_MAX_SHADOW_CASCADES affects PerView size and breaks materials.");

	BufferInterfaceBlock const& UibGenerator::getPerViewUib() noexcept {
		using Type = BufferInterfaceBlock::Type;

		static BufferInterfaceBlock const uib = BufferInterfaceBlock::Builder()
			.name(PerViewUib::_name)
			.add({
			{ "viewFromWorldMatrix",    0, Type::MAT4,   Precision::HIGH},
			{ "worldFromViewMatrix",    0, Type::MAT4,   Precision::HIGH},
			{ "clipFromViewMatrix",     0, Type::MAT4,   Precision::HIGH},
			{ "viewFromClipMatrix",     0, Type::MAT4,   Precision::HIGH},
			{ "clipFromWorldMatrix",    CONFIG_MAX_STEREOSCOPIC_EYES,
										   Type::MAT4,   Precision::HIGH},
			{ "worldFromClipMatrix",    0, Type::MAT4,   Precision::HIGH},
			{ "userWorldFromWorldMatrix",0,Type::MAT4,   Precision::HIGH},
			{ "clipTransform",          0, Type::FLOAT4, Precision::HIGH},

			{ "clipControl",            0, Type::FLOAT2, Precision::HIGH},
			{ "time",                   0, Type::FLOAT,  Precision::HIGH},
			{ "temporalNoise",          0, Type::FLOAT,  Precision::HIGH},
			{ "userTime",               0, Type::FLOAT4, Precision::HIGH},

			// ------------------------------------------------------------------------------------
			// values below should only be accessed in surface materials
			// ------------------------------------------------------------------------------------

			{ "resolution",             0, Type::FLOAT4, Precision::HIGH},
			{ "logicalViewportScale",   0, Type::FLOAT2, Precision::HIGH},
			{ "logicalViewportOffset",  0, Type::FLOAT2, Precision::HIGH},

			{ "lodBias",                0, Type::FLOAT, Precision::DEFAULT},
			{ "refractionLodOffset",    0, Type::FLOAT, Precision::DEFAULT},
			{ "derivativesScale",       0, Type::FLOAT2                  },

			{ "oneOverFarMinusNear",    0, Type::FLOAT,  Precision::HIGH},
			{ "nearOverFarMinusNear",   0, Type::FLOAT,  Precision::HIGH},
			{ "cameraFar",              0, Type::FLOAT,  Precision::HIGH},
			{ "exposure",               0, Type::FLOAT,  Precision::HIGH}, // high precision to work around #3602 (qualcom),
			{ "ev100",                  0, Type::FLOAT,  Precision::DEFAULT},
			{ "needsAlphaChannel",      0, Type::FLOAT,  Precision::DEFAULT},

			// AO
			{ "aoSamplingQualityAndEdgeDistance", 0, Type::FLOAT         },
			{ "aoBentNormals",          0, Type::FLOAT                   },

			// ------------------------------------------------------------------------------------
			// Dynamic Lighting [variant: DYN]
			// ------------------------------------------------------------------------------------
			{ "zParams",                0, Type::FLOAT4                  },
			{ "fParams",                0, Type::UINT3                   },
			{ "lightChannels",          0, Type::INT                     },
			{ "froxelCountXY",          0, Type::FLOAT2                  },

			{ "iblLuminance",           0, Type::FLOAT,  Precision::DEFAULT},
			{ "iblRoughnessOneLevel",   0, Type::FLOAT,  Precision::DEFAULT},
			{ "iblSH",                  9, Type::FLOAT3                  },

			// ------------------------------------------------------------------------------------
			// Directional Lighting [variant: DIR]
			// ------------------------------------------------------------------------------------
			{ "lightDirection",         0, Type::FLOAT3, Precision::HIGH},
			{ "padding0",               0, Type::FLOAT                   },
			{ "lightColorIntensity",    0, Type::FLOAT4, Precision::DEFAULT},
			{ "sun",                    0, Type::FLOAT4, Precision::DEFAULT},
			{ "shadowFarAttenuationParams", 0, Type::FLOAT2, Precision::HIGH },

			// ------------------------------------------------------------------------------------
			// Directional light shadowing [variant: SRE | DIR]
			// ------------------------------------------------------------------------------------
			{ "directionalShadows",       0, Type::INT                      },
			{ "ssContactShadowDistance",  0, Type::FLOAT                    },

			{ "cascadeSplits",             0, Type::FLOAT4, Precision::HIGH },
			{ "cascades",                  0, Type::INT                     },
			{ "shadowPenumbraRatioScale",  0, Type::FLOAT                   },
			{ "lightFarAttenuationParams", 0, Type::FLOAT2, Precision::HIGH },

			// ------------------------------------------------------------------------------------
			// VSM shadows [variant: VSM]
			// ------------------------------------------------------------------------------------
			{ "vsmExponent",             0, Type::FLOAT                  },
			{ "vsmDepthScale",           0, Type::FLOAT                  },
			{ "vsmLightBleedReduction",  0, Type::FLOAT                  },
			{ "shadowSamplingType",      0, Type::UINT                   },

			// ------------------------------------------------------------------------------------
			// Fog [variant: FOG]
			// ------------------------------------------------------------------------------------
			{ "fogDensity",              0, Type::FLOAT3,Precision::HIGH},
			{ "fogStart",                0, Type::FLOAT, Precision::HIGH},
			{ "fogMaxOpacity",           0, Type::FLOAT, Precision::DEFAULT},
			{ "fogMinMaxMip",            0, Type::UINT,  Precision::HIGH },
			{ "fogHeightFalloff",        0, Type::FLOAT, Precision::HIGH},
			{ "fogCutOffDistance",       0, Type::FLOAT, Precision::HIGH},
			{ "fogColor",                0, Type::FLOAT3, Precision::DEFAULT},
			{ "fogColorFromIbl",         0, Type::FLOAT, Precision::DEFAULT},
			{ "fogInscatteringStart",    0, Type::FLOAT, Precision::HIGH},
			{ "fogInscatteringSize",     0, Type::FLOAT, Precision::DEFAULT},
			{ "fogOneOverFarMinusNear",  0, Type::FLOAT, Precision::HIGH },
			{ "fogNearOverFarMinusNear", 0, Type::FLOAT, Precision::HIGH },
			{ "fogFromWorldMatrix",      0, Type::MAT3, Precision::HIGH},

			// ------------------------------------------------------------------------------------
			// Screen-space reflections [variant: SSR (i.e.: VSM | SRE)]
			// ------------------------------------------------------------------------------------
			{ "ssrReprojection",         0, Type::MAT4,  Precision::HIGH },
			{ "ssrUvFromViewMatrix",     0, Type::MAT4,  Precision::HIGH },
			{ "ssrThickness",            0, Type::FLOAT                  },
			{ "ssrBias",                 0, Type::FLOAT                  },
			{ "ssrDistance",             0, Type::FLOAT                  },
			{ "ssrStride",               0, Type::FLOAT                  },

			// --------------------------------------------------------------------------------------------
			// user defined global variables
			// --------------------------------------------------------------------------------------------
			{ "custom",                  4, Type::FLOAT4, Precision::HIGH},

			// --------------------------------------------------------------------------------------------
			// for feature level 0 / es2 usage
			// --------------------------------------------------------------------------------------------
			{ "rec709",                  0, Type::INT,  Precision::DEFAULT},
			{ "es2Reserved0",            0, Type::FLOAT                  },
			{ "es2Reserved1",            0, Type::FLOAT                  },
			{ "es2Reserved2",            0, Type::FLOAT                  },

			// bring PerViewUib to 2 KiB
			{ "reserved", sizeof(PerViewUib::reserved) / 16, Type::FLOAT4 }
				})
			.build();

		return uib;
	}

	BufferInterfaceBlock const& UibGenerator::getPerRenderableUib() noexcept {
		static BufferInterfaceBlock const uib = BufferInterfaceBlock::Builder()
			.name(PerRenderableUib::_name)
			.add({ { "data", CONFIG_MAX_INSTANCES, BufferInterfaceBlock::Type::STRUCT, {},
					"PerRenderableData", sizeof(PerRenderableData), "CONFIG_MAX_INSTANCES" } })
			.build();
		return uib;
	}

	BufferInterfaceBlock const& UibGenerator::getLightsUib() noexcept {
		static BufferInterfaceBlock const uib = BufferInterfaceBlock::Builder()
			.name(LightsUib::_name)
			.add({ { "lights", CONFIG_MAX_LIGHT_COUNT,
					BufferInterfaceBlock::Type::MAT4, Precision::HIGH } })
			.build();
		return uib;
	}

	BufferInterfaceBlock const& UibGenerator::getShadowUib() noexcept {
		static BufferInterfaceBlock const uib = BufferInterfaceBlock::Builder()
			.name(ShadowUib::_name)
			.add({ { "shadows", CONFIG_MAX_SHADOWMAPS,
					BufferInterfaceBlock::Type::STRUCT, {},
					"ShadowData", sizeof(ShadowUib::ShadowData) } })
			.build();
		return uib;
	}

	BufferInterfaceBlock const& UibGenerator::getPerRenderableBonesUib() noexcept {
		static BufferInterfaceBlock const uib = BufferInterfaceBlock::Builder()
			.name(PerRenderableBoneUib::_name)
			.add({ { "bones", CONFIG_MAX_BONE_COUNT,
					BufferInterfaceBlock::Type::STRUCT, {},
					"BoneData", sizeof(PerRenderableBoneUib::BoneData) } })
			.build();
		return uib;
	}

	BufferInterfaceBlock const& UibGenerator::getPerRenderableMorphingUib() noexcept {
		static BufferInterfaceBlock const uib = BufferInterfaceBlock::Builder()
			.name(PerRenderableMorphingUib::_name)
			.add({ { "weights", CONFIG_MAX_MORPH_TARGET_COUNT,
					BufferInterfaceBlock::Type::FLOAT4 } })
			.build();
		return uib;
	}

	BufferInterfaceBlock const& UibGenerator::getFroxelRecordUib() noexcept {
		static BufferInterfaceBlock const uib = BufferInterfaceBlock::Builder()
			.name(FroxelRecordUib::_name)
			.add({ { "records", 1024, BufferInterfaceBlock::Type::UINT4, Precision::HIGH } })
			.build();
		return uib;
	}

	BufferInterfaceBlock const& UibGenerator::getFroxelsUib() noexcept {
		static BufferInterfaceBlock const uib = BufferInterfaceBlock::Builder()
			.name(FroxelsUib::_name)
			.add({ { "records", 1024, BufferInterfaceBlock::Type::UINT4, Precision::HIGH,
					{}, {}, "CONFIG_FROXEL_BUFFER_HEIGHT"} })
			.build();
		return uib;
	}

} // namespace filament
