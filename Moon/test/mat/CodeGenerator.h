#pragma once

#include "MaterialInfo.h"
#include "UibGenerator.h"

#include "MaterialBuilder.h"

#include "MaterialEnums.h"
#include "EngineEnums.h"
#include "SamplerInterfaceBlock.h"//<private/filament/>
#include "BufferInterfaceBlock.h"//<private/filament/>
//#include <private/filament/SubpassInfo.h>
#include "test/DriverEnums.h"//<private/filament/Variant.h>
#include "Variant.h"



#include "test/utils/FixedCapacityVector.h"
#include "test/utils/sstream.h"

#include <exception>
#include <iosfwd>
#include <string>
#include <variant>

#include <stdint.h>

namespace TEST {

	class  CodeGenerator {



	public:
		CodeGenerator() noexcept {

		}



		// insert a separator (can be a new line)
		static sstream& generateSeparator(sstream& out);

		// generate prolog for the given shader
		sstream& generateProlog(sstream& out, ShaderStage stage,
			MaterialInfo const& material, Variant v) const;

		static sstream& generateEpilog(sstream& out);

		static sstream& generateCommonTypes(sstream& out, ShaderStage stage);

		// generate common functions for the given shader
		static sstream& generateCommon(sstream& out, ShaderStage stage);
		static sstream& generatePostProcessCommon(sstream& out, ShaderStage type);
		static sstream& generateCommonMaterial(sstream& out, ShaderStage type);

		static sstream& generateFog(sstream& out, ShaderStage type);

		// generate the shader's main()
		static sstream& generateShaderMain(sstream& out, ShaderStage stage);
		static sstream& generatePostProcessMain(sstream& out, ShaderStage type);

		// generate the shader's code for the lit shading model
		static sstream& generateShaderLit(sstream& out, ShaderStage type,
			Variant variant, Shading shading, bool customSurfaceShading);

		// generate the shader's code for the unlit shading model
		static sstream& generateShaderUnlit(sstream& out, ShaderStage type,
			Variant variant, bool hasShadowMultiplier);

		// generate the shader's code for the screen-space reflections
		static sstream& generateShaderReflections(sstream& out, ShaderStage type);

		// generate declarations for custom interpolants
		static sstream& generateVariable(sstream& out, ShaderStage stage,
			const MaterialBuilder::CustomVariable& variable, size_t index);

		// generate declarations for non-custom "in" variables
		sstream& generateShaderInputs(sstream& out, ShaderStage type,
			const AttributeBitset& attributes, Interpolation interpolation,
			MaterialBuilder::PushConstantList const& pushConstants) const;
		static sstream& generatePostProcessInputs(sstream& out, ShaderStage type);

		// generate declarations for custom output variables
		sstream& generateOutput(sstream& out, ShaderStage type,
			const utils::CString& name, size_t index,
			MaterialBuilder::VariableQualifier qualifier,
			MaterialBuilder::Precision precision,
			MaterialBuilder::OutputType outputType) const;

		// generate no-op shader for depth prepass
		static sstream& generateDepthShaderMain(sstream& out, ShaderStage type);

		// generate samplers
		sstream& generateSamplers(sstream& out,
			DescriptorSetBindingPoints set,
			SamplerInterfaceBlock::SamplerInfoList const& list) const;

		sstream& generateSamplers(sstream& out,
			DescriptorSetBindingPoints set,
			const SamplerInterfaceBlock& sib) const {
			return generateSamplers(out, set, sib.getSamplerInfoList());
		}

		// generate subpass
		//static sstream& generateSubpass(sstream& out,
			//SubpassInfo subpass);

		// generate uniforms
		sstream& generateUniforms(sstream& out, ShaderStage stage,
			DescriptorSetBindingPoints set,
			descriptor_binding_t binding,
			const BufferInterfaceBlock& uib) const;

		// generate buffers
		sstream& generateBuffers(sstream& out,
			MaterialInfo::BufferContainer const& buffers) const;

		// generate an interface block
		sstream& generateBufferInterfaceBlock(sstream& out, ShaderStage stage,
			DescriptorSetBindingPoints set,
			descriptor_binding_t binding,
			const BufferInterfaceBlock& uib) const;

		// generate material properties getters
		static sstream& generateMaterialProperty(sstream& out,
			MaterialBuilder::Property property, bool isSet);

		sstream& generateQualityDefine(sstream& out, ShaderQuality quality) const;

		static sstream& generateDefine(sstream& out, const char* name, bool value);
		static sstream& generateDefine(sstream& out, const char* name, uint32_t value);
		static sstream& generateDefine(sstream& out, const char* name, const char* string);
		static sstream& generateIndexedDefine(sstream& out, const char* name,
			uint32_t index, uint32_t value);

		sstream& generateSpecializationConstant(sstream& out,
			const char* name, uint32_t id, std::variant<int, float, bool> value) const;

		sstream& generatePushConstants(sstream& out,
			MaterialBuilder::PushConstantList const& pushConstants,
			size_t const layoutLocation) const;

		static sstream& generatePostProcessGetters(sstream& out, ShaderStage type);
		static sstream& generateGetters(sstream& out, ShaderStage stage);
		static sstream& generateParameters(sstream& out, ShaderStage type);

		static void fixupExternalSamplers(
			std::string& shader, SamplerInterfaceBlock const& sib,
			FeatureLevel featureLevel) noexcept;

		// These constants must match the equivalent in MetalState.h.
		// These values represent the starting index for uniform, ssbo, and sampler group [[buffer(n)]]
		// bindings. See the chart at the top of MetalState.h.
		static constexpr uint32_t METAL_PUSH_CONSTANT_BUFFER_INDEX = 20u;
		static constexpr uint32_t METAL_DESCRIPTOR_SET_BINDING_START = 21u;
		static constexpr uint32_t METAL_DYNAMIC_OFFSET_BINDING = 25u;

		uint32_t getUniqueSamplerBindingPoint() const noexcept {
			return mUniqueSamplerBindingPoint++;
		}

		uint32_t getUniqueUboBindingPoint() const noexcept {
			return mUniqueUboBindingPoint++;
		}

		uint32_t getUniqueSsboBindingPoint() const noexcept {
			return mUniqueSsboBindingPoint++;
		}

	private:
		Precision getDefaultPrecision(ShaderStage stage) const;
		Precision getDefaultUniformPrecision() const;

		sstream& generateInterfaceFields(sstream& out,
			utils::FixedCapacityVector<BufferInterfaceBlock::FieldInfo> const& infos,
			Precision defaultPrecision) const;

		sstream& generateUboAsPlainUniforms(sstream& out, ShaderStage stage,
			const BufferInterfaceBlock& uib) const;

		static const char* getUniformPrecisionQualifier(UniformType type,
			Precision precision,
			Precision uniformPrecision,
			Precision defaultPrecision) noexcept;

		// return type name of sampler  (e.g.: "sampler2D")
		char const* getSamplerTypeName(SamplerType type,
			SamplerFormat format, bool multisample) const noexcept;

		// return name of the material property (e.g.: "ROUGHNESS")
		static char const* getConstantName(MaterialBuilder::Property property) noexcept;

		static char const* getPrecisionQualifier(Precision precision) noexcept;

		// return type (e.g.: "vec3", "vec4", "float")
		static char const* getTypeName(UniformType type) noexcept;

		// return type name of uniform Field (e.g.: "vec3", "vec4", "float")
		static char const* getUniformTypeName(BufferInterfaceBlock::FieldInfo const& info) noexcept;

		// return type name of output  (e.g.: "vec3", "vec4", "float")
		static char const* getOutputTypeName(MaterialBuilder::OutputType type) noexcept;

		// return qualifier for the specified interpolation mode
		static char const* getInterpolationQualifier(Interpolation interpolation) noexcept;

		static bool hasPrecision(BufferInterfaceBlock::Type type) noexcept;

		mutable uint32_t mUniqueSamplerBindingPoint = 0;
		mutable uint32_t mUniqueUboBindingPoint = 0;
		mutable uint32_t mUniqueSsboBindingPoint = 0;
	};

}